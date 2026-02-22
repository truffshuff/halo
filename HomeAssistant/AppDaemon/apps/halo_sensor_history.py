"""
halo_sensor_history.py – AppDaemon app
=======================================
Listens for ``esphome.aqi_history_batch`` events fired by the Halo device
when it reconnects after an outage and backfills all 12 sensor statistics into
the Home Assistant recorder database via ``recorder.import_statistics``.

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
        target_device = self.args.get("device_filter", None)
        self._device_filter = target_device

        self.listen_event(self.handle_batch, "esphome.aqi_history_batch")
        self.log(
            f"Halo sensor history listener started "
            f"(device_filter={target_device or 'any'})"
        )

    def handle_batch(self, event_name, data, kwargs):
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

        imported_total = 0

        for key, statistic_id, unit in SENSORS:
            stats = []
            for r in readings:
                val = r.get(key)
                if val is None:
                    continue
                try:
                    val = float(val)
                    ts  = int(r["ts"])
                    dt  = datetime.fromtimestamp(ts, tz=timezone.utc)
                    stats.append(
                        {
                            "start": dt.isoformat(),
                            "mean":  val,
                            "min":   val,
                            "max":   val,
                        }
                    )
                except Exception as exc:
                    self.log(
                        f"[{device}] Skipping malformed entry for key '{key}': {exc}",
                        level="WARNING",
                    )
                    continue

            if not stats:
                self.log(
                    f"[{device}] No valid values for '{key}' ({statistic_id}) – skipping.",
                    level="DEBUG",
                )
                continue

            try:
                self.call_service(
                    "recorder/import_statistics",
                    statistic_id=statistic_id,
                    source="recorder",
                    unit_of_measurement=unit,
                    has_mean=True,
                    has_sum=False,
                    stats=stats,
                )
                self.log(
                    f"[{device}]   ✓ {len(stats):>4d} pts → {statistic_id}"
                )
                imported_total += len(stats)
            except Exception as exc:
                self.log(
                    f"[{device}]   ✗ Failed to import {statistic_id}: {exc}",
                    level="ERROR",
                )

        self.log(
            f"[{device}] Batch {batch_idx} complete – "
            f"{imported_total} stat points imported across {len(SENSORS)} sensors."
        )
