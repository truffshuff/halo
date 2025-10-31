# Halo V1 Configuration Guide - Substitution-Based Configuration

## Quick Start

Features are controlled via **substitutions** at the top of `halo-v1-79e384.yaml`, with package includes organized clearly in the packages section.

### Step 1: Edit Feature Toggles (Substitutions)

Open `halo-v1-79e384.yaml` and find the substitutions section (lines 42-71). Set each feature to `"true"` or `"false"`:

```yaml
substitutions:
  # Air Quality Monitoring (AirQ)
  airq_enabled: "true"          # ← Set to "true" or "false"

  # BLE Presence Detection
  ble_enabled: "false"          # ← Set to "true" or "false"

  # WiFi Info Display
  wifi_enabled: "false"         # ← Set to "true" or "false"

  # Weather Display
  weather_enabled: "false"      # ← Set to "true" or "false"

  # WireGuard VPN (requires wifi_enabled = true)
  wireguard_enabled: "false"    # ← Set to "true" or "false"
```

### Step 2: Update LVGL Pages Combo

Still in substitutions, set `lvgl_pages_combo` to match your feature configuration:

```yaml
# "clock-only":        No features enabled
# "airq-only":         AirQ only
# "wifi-only":         WiFi only
# "weather-only":      Weather only
# "airq-wifi":         AirQ + WiFi
# "airq-weather":      AirQ + Weather
# "wifi-weather":      WiFi + Weather
# "airq-wifi-weather": AirQ + WiFi + Weather

lvgl_pages_combo: "airq-only"  # ← Match your features above
```

### Step 3: Synchronize Packages Section

Scroll down to the packages section (lines 90-143) and uncomment/comment includes to match your substitutions:

**For each enabled feature:** Uncomment the `-core.yaml` or `-display.yaml` line and comment the `-stubs.yaml` line
**For each disabled feature:** Comment the `-core.yaml` or `-display.yaml` line and uncomment the `-stubs.yaml` line

Example for AirQ enabled, BLE disabled:
```yaml
# Air Quality Monitoring (AirQ)
airq: !include packages/airq-core.yaml         # ← UNCOMMENTED
# airq: !include packages/airq-stubs.yaml

# BLE Presence Detection
# ble: !include packages/ble-core-simplified.yaml
ble: !include packages/ble-stubs.yaml          # ← UNCOMMENTED
```

### Step 4: Update LVGL Pages Line

Still in packages section, uncomment the LVGL pages line matching your `lvgl_pages_combo` and comment others:

```yaml
lvgl_pages: !include packages/lvgl-pages-airq-only.yaml     # ← UNCOMMENTED
# lvgl_pages: !include packages/lvgl-pages-clock-only.yaml
# lvgl_pages: !include packages/lvgl-pages-wifi-only.yaml
```

### Step 5: Compile!

Done! Compile in ESPHome Dashboard.

## Examples

### Example 1: Clock Only (Minimal, Memory Efficient)

```yaml
substitutions:
  airq_enabled: "false"
  ble_enabled: "false"
  wifi_enabled: "false"
  weather_enabled: "false"
  wireguard_enabled: "false"

  lvgl_pages_combo: "clock-only"
```

**Result**: ~45KB RAM, just the vertical clock

### Example 2: AirQ + WiFi

```yaml
substitutions:
  airq_enabled: "true"
  ble_enabled: "false"
  wifi_enabled: "true"
  weather_enabled: "false"
  wireguard_enabled: "false"

  lvgl_pages_combo: "airq-wifi"
```

**Result**: AirQ page + WiFi page, ~125KB RAM

### Example 3: Weather + BLE (Revolutionary!)

```yaml
substitutions:
  airq_enabled: "false"
  ble_enabled: "true"
  wifi_enabled: "false"
  weather_enabled: "true"
  wireguard_enabled: "false"

  lvgl_pages_combo: "weather-only"
```

**Result**: Weather forecast + BLE presence tracking, ~108KB RAM

### Example 4: Everything!

```yaml
substitutions:
  airq_enabled: "true"
  ble_enabled: "false"         # Can't fit with all others
  wifi_enabled: "true"
  weather_enabled: "true"
  wireguard_enabled: "true"

  lvgl_pages_combo: "airq-wifi-weather"
```

**Result**: All pages enabled, ~140KB RAM (BLE won't fit)

## Memory Usage Reference

| Feature | Size |
|---------|------|
| Clock (always included) | ~15KB |
| AirQ sensors + page | ~20KB |
| BLE tracking | ~60KB |
| WiFi display | ~5KB |
| Weather sensors | ~75KB |
| Weather + HA Actions | ~3KB |
| WireGuard display | ~2KB |

## Important Notes

### Time Update Script

The time update script is **automatically selected** based on `airq_enabled`:
- `airq_enabled: "true"` → Uses `time-update-basic.yaml`
- `airq_enabled: "false"` → Uses `time-update-clock-only.yaml`

You don't need to change this manually.

### LVGL Pages Must Match Features

If your features don't match your `lvgl_pages_combo`, you'll get compilation errors.

**Correct** ✅:
```yaml
airq_enabled: "true"
wifi_enabled: "false"
lvgl_pages_combo: "airq-only"
```

**Wrong** ❌:
```yaml
airq_enabled: "true"
wifi_enabled: "false"
lvgl_pages_combo: "airq-wifi"  # WiFi pages included but wifi_enabled is false!
```

### Memory Statistics

The memory stats package is included by default (memory-stats). To disable it:

In the packages section, comment out:
```yaml
# memory_stats: !include packages/memory-stats.yaml
```

## Troubleshooting

### "Component not found" Error

Make sure your `lvgl_pages_combo` matches the features you enabled:

```
Failed config
id: [source /config/esphome/packages/lvgl-pages-airq-only.yaml:...]
  Component not found: id. AirQ_page
```

This means you selected `lvgl_pages_combo: "airq-only"` but `airq_enabled: "false"`.

**Solution**: Either:
1. Enable AirQ: `airq_enabled: "true"`, or
2. Change pages: `lvgl_pages_combo: "clock-only"`

### Stubs Warning

You might see warnings like:

```
WARNING: "AQI Color" LED effect requires AirQ to be enabled
```

This is just informational. Stubs prevent crashes by providing safe defaults.

## File Structure

```
packages/
├── airq-true.yaml            ← Loads airq-core.yaml
├── airq-false.yaml           ← Loads airq-stubs.yaml
├── ble-true.yaml             ← Loads ble-core-simplified.yaml
├── ble-false.yaml            ← Loads ble-stubs.yaml
├── wifi-true.yaml            ← Loads wifi-display.yaml
├── wifi-false.yaml           ← Loads wifi-stubs.yaml
├── weather-true.yaml         ← Loads weather-ha-actions.yaml
├── weather-false.yaml        ← Loads weather-stubs.yaml
├── wireguard-true.yaml       ← Loads wireguard.yaml
├── wireguard-false.yaml      ← Loads wireguard-stubs.yaml
├── time-update-true.yaml     ← Loads time-update-basic.yaml
├── time-update-false.yaml    ← Loads time-update-clock-only.yaml
└── lvgl-pages-*.yaml         ← Actual page layouts
```

## Next Steps

1. Edit substitutions in `halo-v1-79e384.yaml`
2. Update `lvgl_pages_combo` to match
3. Compile in ESPHome Dashboard
4. Done! No manual include management needed
