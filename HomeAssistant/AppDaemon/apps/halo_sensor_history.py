"""
halo_sensor_history.py – AppDaemon app
=======================================
Listens for ``esphome.aqi_history_batch`` events fired by the Halo device
and backfills all 12 sensor statistics into the Home Assistant recorder DB.

WHY WEBSOCKET DIRECTLY?
    ``recorder.import_statistics`` was removed as a HA *service call* in HA
    2026.x, but the underlying WebSocket command ``recorder/import_statistics``
    still exists.  AppDaemon's ``call_service()`` only wraps the ``call_service``
    WS command, so we talk to the HA WebSocket API directly using ``aiohttp``
    (already installed as an AppDaemon dependency).

SETUP
-----
1. Copy this file to your AppDaemon ``apps/`` directory.
2. Add the entry from ``halo_sensor_history.yaml`` (alongside this file) to
   your ``apps.yaml`` (or place the yaml file in the apps dir if using
   AppDaemon's per-app file loading).
3. Restart AppDaemon.

SENSOR MAP
----------
JSON key  →  HA statistic_id                                        unit
---------     -------------------------------------------------------  ------
q         →  sensor.halo_v1_79e35c_computed_aqi                       (aqi)
c2        →  sensor.halo_v1_79e35c_co2                                 ppm
n2        →  sensor.halo_v1_79e35c_nitrogen_dioxide                    ppm
co        →  sensor.halo_v1_79e35c_carbon_monoxide                     ppm
h2        →  sensor.halo_v1_79e35c_hydrogen                            ppm
c4        →  sensor.halo_v1_79e35c_methane                             ppm
et        →  sensor.halo_v1_79e35c_ethanol                             ppm
n3        →  sensor.halo_v1_79e35c_ammonia                             ppm
p1        →  sensor.halo_v1_79e35c_pm_1_m_weight_concentration         µg/m³
p25       →  sensor.halo_v1_79e35c_pm_2_5_m_weight_concentration       µg/m³
p4        →  sensor.halo_v1_79e35c_pm_4_m_weight_concentration         µg/m³
p10       →  sensor.halo_v1_79e35c_pm_10_m_weight_concentration        µg/m³

NOTE: ``unit_of_measurement`` must exactly match what HA has already recorded
for each sensor.  If a sensor's unit in HA doesn't match, import will silently
fail.  Verify via Developer Tools → Statistics → <sensor> → ⓘ.
If you need to override units for a sensor, edit the SENSORS list below.
"""

import json
import os
import sqlite3
import struct
import hashlib
from datetime import datetime, timezone

import aiohttp
import appdaemon.plugins.hass.hassapi as hass

# ---------------------------------------------------------------------------
# Sensor map: (json_key, statistic_id, unit_of_measurement)
# Update statistic_id / unit if your entity names differ.
# ---------------------------------------------------------------------------
SENSORS = [
    ("q",   "sensor.halo_v1_79e35c_computed_aqi",                ""),
    ("c2",  "sensor.halo_v1_79e35c_co2",                         "ppm"),
    ("n2",  "sensor.halo_v1_79e35c_nitrogen_dioxide",            "ppm"),
    ("co",  "sensor.halo_v1_79e35c_carbon_monoxide",             "ppm"),
    ("h2",  "sensor.halo_v1_79e35c_hydrogen",                    "ppm"),
    ("c4",  "sensor.halo_v1_79e35c_methane",                     "ppm"),
    ("et",  "sensor.halo_v1_79e35c_ethanol",                     "ppm"),
    ("n3",  "sensor.halo_v1_79e35c_ammonia",                     "ppm"),
    ("p1",  "sensor.halo_v1_79e35c_pm_1_m_weight_concentration", "µg/m³"),
    ("p25", "sensor.halo_v1_79e35c_pm_2_5_m_weight_concentration","µg/m³"),
    ("p4",  "sensor.halo_v1_79e35c_pm_4_m_weight_concentration", "µg/m³"),
    ("p10", "sensor.halo_v1_79e35c_pm_10_m_weight_concentration","µg/m³"),
]


class HaloSensorHistory(hass.Hass):

    def initialize(self):
        self._device_filter = self.args.get("device_filter", None)
        self._ha_url        = self.args["ha_url"].rstrip("/")
        self._ha_token      = self.args["ha_token"]

        # Resolve the recorder DB path. AppDaemon add-ons mount the HA config
        # directory at /homeassistant, not /config. Try the configured path first,
        # then fall back to common locations automatically.
        configured_path = self.args.get("ha_db_path", None)
        candidates = []
        if configured_path:
            candidates.append(configured_path)
        candidates += [
            "/homeassistant/home-assistant_v2.db",  # HAOS / Supervised add-on
            "/config/home-assistant_v2.db",          # Docker with /config mount
            "/usr/share/hassio/homeassistant/home-assistant_v2.db",
        ]
        self._ha_db_path = None
        for path in candidates:
            if os.path.exists(path):
                self._ha_db_path = path
                break
        if self._ha_db_path is None:
            self._ha_db_path = configured_path or candidates[0]
            self.log(
                f"WARNING: recorder DB not found at any of {candidates}. "
                f"Set ha_db_path in halo_sensor_history.yaml to the correct path. "
                f"Per-minute history writes will be skipped.",
                level="WARNING",
            )
        else:
            self.log(f"Recorder DB resolved to: {self._ha_db_path}")
        # Convert http(s) base URL to ws(s) WebSocket URL
        self._ws_url = (
            self._ha_url
            .replace("https://", "wss://", 1)
            .replace("http://",  "ws://",  1)
            + "/api/websocket"
        )

        self.listen_event(self.handle_batch, "esphome.aqi_history_batch")
        self.log(
            f"Halo sensor history listener started "
            f"(device_filter={self._device_filter or 'any'}, ws={self._ws_url}, "
            f"db={self._ha_db_path})"
        )

    async def handle_batch(self, event_name, data, kwargs):
        device = data.get("device", "unknown")

        # Optionally filter to a specific device name (set in apps.yaml)
        if self._device_filter and device != self._device_filter:
            self.log(
                f"Ignoring batch from '{device}' (filter='{self._device_filter}')",
                level="DEBUG",
            )
            return

        batch_idx = data.get("batch", "?")
        total     = data.get("total", "?")
        count     = data.get("count", "?")
        readings_raw = data.get("readings", "[]")

        self.log(
            f"[{device}] Received batch {batch_idx} – "
            f"{count} entries (total flush: {total})"
        )

        try:
            readings = json.loads(readings_raw)
        except Exception as exc:
            self.log(f"[{device}] Failed to parse readings JSON: {exc}", level="ERROR")
            return

        if not readings:
            self.log(f"[{device}] Empty readings list – nothing to import.", level="WARNING")
            return

        # Build per-sensor stats lists from the batch.
        # recorder/import_statistics requires hourly buckets: timestamps must
        # be floored to the top of the hour (minutes=0, seconds=0).
        # Multiple readings within the same hour are averaged together.
        sensor_stats = {}
        for key, statistic_id, unit in SENSORS:
            # bucket_key -> {sum, count, min, max}
            buckets: dict = {}
            for r in readings:
                val = r.get(key)
                if val is None:
                    continue
                try:
                    val = float(val)
                    dt  = datetime.fromtimestamp(int(r["ts"]), tz=timezone.utc)
                    # Floor to top of hour
                    hour_dt = dt.replace(minute=0, second=0, microsecond=0)
                    iso     = hour_dt.isoformat()
                    if iso not in buckets:
                        buckets[iso] = {"sum": 0.0, "count": 0, "min": val, "max": val}
                    b = buckets[iso]
                    b["sum"]   += val
                    b["count"] += 1
                    if val < b["min"]: b["min"] = val
                    if val > b["max"]: b["max"] = val
                except Exception as exc:
                    self.log(
                        f"[{device}] Skipping malformed entry for key '{key}': {exc}",
                        level="WARNING",
                    )
            if buckets:
                sensor_stats[key] = [
                    {
                        "start": iso,
                        "mean":  b["sum"] / b["count"],
                        "min":   b["min"],
                        "max":   b["max"],
                    }
                    for iso, b in sorted(buckets.items())
                ]

        if not sensor_stats:
            self.log(f"[{device}] No valid sensor data in batch – skipping.", level="WARNING")
            return

        try:
            imported_total = await self._import_via_websocket(device, sensor_stats)
        except Exception as exc:
            self.log(
                f"[{device}] WebSocket import failed for batch {batch_idx}: {exc}",
                level="ERROR",
            )
            imported_total = 0

        # Also write per-minute states directly to the recorder SQLite DB so the
        # History panel shows the full sub-hourly resolution, not just hourly stats.
        try:
            loop = self.AD.loop
            history_total = await loop.run_in_executor(
                None, self._import_via_sqlite, device, readings
            )
        except Exception as exc:
            self.log(
                f"[{device}] SQLite history import failed for batch {batch_idx}: {exc}",
                level="ERROR",
            )
            history_total = 0

        self.log(
            f"[{device}] Batch {batch_idx} complete – "
            f"{imported_total} hourly stat pts + {history_total} per-min history pts "
            f"across {len(sensor_stats)} sensors."
        )

    async def _import_via_websocket(self, device: str, sensor_stats: dict) -> int:
        """Send recorder/import_statistics directly over the HA WebSocket API.

        recorder.import_statistics was removed as a *service* in HA 2026.x
        but the raw WebSocket command type still works.
        """
        imported_total = 0
        msg_id = 1  # monotonically increasing per WS session

        async with aiohttp.ClientSession() as session:
            async with session.ws_connect(self._ws_url) as ws:

                # HA WebSocket authentication handshake
                auth_req = await ws.receive_json()
                if auth_req.get("type") != "auth_required":
                    raise RuntimeError(f"Unexpected WS opening message: {auth_req}")

                await ws.send_json({"type": "auth", "access_token": self._ha_token})
                auth_resp = await ws.receive_json()
                if auth_resp.get("type") != "auth_ok":
                    raise RuntimeError(f"HA WebSocket auth failed: {auth_resp}")

                # Send recorder/import_statistics for each sensor
                for key, statistic_id, unit in SENSORS:
                    stats = sensor_stats.get(key)
                    if not stats:
                        continue

                    await ws.send_json({
                        "id":   msg_id,
                        "type": "recorder/import_statistics",
                        # HA 2026.x moved all descriptor fields into "metadata"
                        "metadata": {
                            "statistic_id":        statistic_id,
                            "source":              "recorder",
                            "unit_of_measurement": unit,
                            "has_mean":            True,
                            "has_sum":             False,
                            "name":                None,
                        },
                        "stats": stats,
                    })

                    result = await ws.receive_json()
                    if result.get("success") is False:
                        self.log(
                            f"[{device}]   ✗ {statistic_id}: "
                            f"{result.get('error', {}).get('message', result)}",
                            level="ERROR",
                        )
                    else:
                        self.log(f"[{device}]   ✓ {len(stats):>4d} pts → {statistic_id}")
                        imported_total += len(stats)

                    msg_id += 1

        return imported_total

    # ------------------------------------------------------------------
    # Write per-minute states directly to the HA recorder SQLite DB so
    # that the History panel shows sub-hourly resolution data.
    #
    # This method is SYNCHRONOUS and must be called via run_in_executor.
    # It uses WAL mode and short transactions to minimise lock contention
    # with the HA recorder process that is also writing to the same DB.
    #
    # Schema notes (HA 2023.4+ / schema >= 43):
    #   states_meta(metadata_id, entity_id)
    #   state_attributes(attributes_id, hash, shared_attrs)
    #   states(state_id, state, last_changed_ts, last_updated_ts,
    #          last_reported_ts, old_state_id, attributes_id,
    #          context_id_bin, context_user_id_bin, context_parent_id_bin,
    #          metadata_id, origin_idx)
    # ------------------------------------------------------------------
    def _import_via_sqlite(self, device: str, readings: list) -> int:
        if not os.path.exists(self._ha_db_path):
            self.log(
                f"[{device}] Recorder DB not found at {self._ha_db_path} – skipping history write.",
                level="WARNING",
            )
            return 0

        inserted_total = 0

        conn = sqlite3.connect(self._ha_db_path, timeout=30, check_same_thread=False)
        try:
            conn.execute("PRAGMA journal_mode=WAL")
            conn.execute("PRAGMA synchronous=NORMAL")

            # Introspect states table columns once
            state_cols = {row[1] for row in conn.execute("PRAGMA table_info(states)")}
            has_metadata_id      = "metadata_id"       in state_cols
            has_last_changed_ts  = "last_changed_ts"    in state_cols
            has_last_reported_ts = "last_reported_ts"   in state_cols
            has_origin_idx       = "origin_idx"         in state_cols
            has_context_bin      = "context_id_bin"     in state_cols

            for key, entity_id, unit in SENSORS:
                # Collect readings for this sensor in timestamp order
                rows = []
                for r in readings:
                    val = r.get(key)
                    if val is None:
                        continue
                    try:
                        rows.append((float(r["ts"]), round(float(val), 4)))
                    except Exception:
                        continue
                if not rows:
                    continue
                rows.sort(key=lambda x: x[0])

                # ---- ensure states_meta entry ----
                if has_metadata_id:
                    conn.execute(
                        "INSERT OR IGNORE INTO states_meta (entity_id) VALUES (?)",
                        (entity_id,),
                    )
                    meta = conn.execute(
                        "SELECT metadata_id FROM states_meta WHERE entity_id = ?",
                        (entity_id,),
                    ).fetchone()
                    if not meta:
                        continue
                    metadata_id = meta[0]
                else:
                    metadata_id = None

                # ---- get/create attributes_id ----
                # Try to reuse the most recent attributes for this entity so
                # the injected states look identical to live ones (same unit, etc.)
                if has_metadata_id:
                    recent_attr = conn.execute(
                        "SELECT attributes_id FROM states WHERE metadata_id = ? "
                        "AND attributes_id IS NOT NULL "
                        "ORDER BY last_updated_ts DESC LIMIT 1",
                        (metadata_id,),
                    ).fetchone()
                else:
                    recent_attr = conn.execute(
                        "SELECT attributes_id FROM states WHERE entity_id = ? "
                        "AND attributes_id IS NOT NULL "
                        "ORDER BY last_updated DESC LIMIT 1",
                        (entity_id,),
                    ).fetchone()

                if recent_attr:
                    attributes_id = recent_attr[0]
                else:
                    # Fallback: create a minimal attributes entry
                    attrs_json = json.dumps({
                        "unit_of_measurement": unit,
                        "state_class": "measurement",
                        "friendly_name": entity_id.replace("sensor.", "").replace("_", " ").title(),
                    })
                    attrs_hash = struct.unpack(">q", hashlib.sha256(attrs_json.encode()).digest()[:8])[0]
                    conn.execute(
                        "INSERT OR IGNORE INTO state_attributes (hash, shared_attrs) VALUES (?, ?)",
                        (attrs_hash, attrs_json),
                    )
                    attr_row = conn.execute(
                        "SELECT attributes_id FROM state_attributes WHERE hash = ?",
                        (attrs_hash,),
                    ).fetchone()
                    attributes_id = attr_row[0] if attr_row else None

                # ---- find old_state_id anchor ----
                # Link our first inserted row to the last existing state before it
                first_ts = rows[0][0]
                if has_metadata_id and has_last_changed_ts:
                    anchor = conn.execute(
                        "SELECT state_id FROM states WHERE metadata_id = ? "
                        "AND last_updated_ts < ? ORDER BY last_updated_ts DESC LIMIT 1",
                        (metadata_id, first_ts),
                    ).fetchone()
                elif has_metadata_id:
                    anchor = conn.execute(
                        "SELECT state_id FROM states WHERE metadata_id = ? "
                        "AND last_updated < ? ORDER BY last_updated DESC LIMIT 1",
                        (metadata_id, first_ts),
                    ).fetchone()
                else:
                    anchor = None
                prev_state_id = anchor[0] if anchor else None

                # ---- insert one state row per reading ----
                inserted_this = 0
                for ts, val in rows:
                    state_str = str(val)
                    ctx_bin   = os.urandom(16)  # random context UUID

                    if has_metadata_id and has_last_changed_ts:
                        extra_cols = ""
                        extra_vals = []
                        if has_last_reported_ts:
                            extra_cols += ", last_reported_ts"
                            extra_vals.append(ts)
                        if has_origin_idx:
                            extra_cols += ", origin_idx"
                            extra_vals.append(0)  # 0 = LOCAL
                        if has_context_bin:
                            extra_cols += ", context_id_bin, context_user_id_bin, context_parent_id_bin"
                            extra_vals += [ctx_bin, None, None]

                        cur = conn.execute(
                            f"INSERT INTO states "
                            f"(state, last_changed_ts, last_updated_ts, "
                            f"old_state_id, attributes_id, metadata_id{extra_cols}) "
                            f"VALUES (?, ?, ?, ?, ?, ?{', ?' * len(extra_vals)})",
                            [state_str, ts, ts, prev_state_id, attributes_id, metadata_id]
                            + extra_vals,
                        )
                    else:
                        # Older schema fallback
                        from datetime import datetime, timezone as _tz
                        dt_str = datetime.fromtimestamp(ts, tz=_tz.utc).isoformat()
                        cur = conn.execute(
                            "INSERT INTO states "
                            "(entity_id, state, last_changed, last_updated, "
                            "old_state_id, attributes_id) "
                            "VALUES (?, ?, ?, ?, ?, ?)",
                            [entity_id, state_str, ts, ts, prev_state_id, attributes_id],
                        )

                    prev_state_id = cur.lastrowid
                    inserted_this += 1

                conn.commit()
                inserted_total += inserted_this
                self.log(
                    f"[{device}]   DB {inserted_this:>4d} rows → {entity_id}"
                )

        except Exception as exc:
            conn.rollback()
            self.log(f"[{device}] SQLite write error: {exc}", level="ERROR")
        finally:
            conn.close()

        return inserted_total
