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
            f"(device_filter={self._device_filter or 'any'}, ws={self._ws_url})"
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

        # Build per-sensor stats lists from the batch
        sensor_stats = {}
        for key, statistic_id, unit in SENSORS:
            stats = []
            for r in readings:
                val = r.get(key)
                if val is None:
                    continue
                try:
                    dt = datetime.fromtimestamp(int(r["ts"]), tz=timezone.utc)
                    stats.append({
                        "start": dt.isoformat(),
                        "mean":  float(val),
                        "min":   float(val),
                        "max":   float(val),
                    })
                except Exception as exc:
                    self.log(
                        f"[{device}] Skipping malformed entry for key '{key}': {exc}",
                        level="WARNING",
                    )
            if stats:
                sensor_stats[key] = stats

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
            return

        self.log(
            f"[{device}] Batch {batch_idx} complete – "
            f"{imported_total} stat points imported across {len(sensor_stats)} sensors."
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
                        "id":                  msg_id,
                        "type":                "recorder/import_statistics",
                        "statistic_id":        statistic_id,
                        "source":              "recorder",
                        "unit_of_measurement": unit,
                        "has_mean":            True,
                        "has_sum":             False,
                        "stats":               stats,
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
