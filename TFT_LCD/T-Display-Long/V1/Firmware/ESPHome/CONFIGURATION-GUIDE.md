# Halo V1 Configuration Guide - Dynamic Substitutions

## Quick Start

All features are now controlled via **substitutions** at the top of `halo-v1-79e384.yaml`. No more commenting/uncommenting!

### Step 1: Edit Feature Toggles

Open `halo-v1-79e384.yaml` and find the substitutions section (around line 42-55). Change features from `"true"` to `"false"` or vice versa:

```yaml
substitutions:
  # Air Quality Monitoring (AirQ)
  airq_enabled: "true"          # ← Change this

  # BLE Presence Detection
  ble_enabled: "false"          # ← Or this

  # WiFi Info Display
  wifi_enabled: "false"         # ← Or this

  # Weather Display
  weather_enabled: "false"      # ← Or this

  # WireGuard VPN (requires wifi_enabled = true)
  wireguard_enabled: "false"    # ← Or this
```

### Step 2: Update LVGL Pages

After changing features, update `lvgl_pages_combo` to match your configuration:

```yaml
# "clock-only":        No features enabled
# "airq-only":         AirQ only
# "wifi-only":         WiFi only
# "weather-only":      Weather only
# "airq-wifi":         AirQ + WiFi
# "airq-weather":      AirQ + Weather
# "wifi-weather":      WiFi + Weather
# "airq-wifi-weather": AirQ + WiFi + Weather

lvgl_pages_combo: "airq-only"  # ← Change to match above
```

### Step 3: Done!

Everything else happens automatically. The correct packages are included based on your feature toggles.

## How It Works

### Feature Toggles → Package Selection

Each feature toggle controls which package is loaded:

| Feature | `"true"` Package | `"false"` Package |
|---------|---|---|
| `airq_enabled` | `airq-core.yaml` | `airq-stubs.yaml` |
| `ble_enabled` | `ble-core-simplified.yaml` | `ble-stubs.yaml` |
| `wifi_enabled` | `wifi-display.yaml` | `wifi-stubs.yaml` |
| `weather_enabled` | `weather-ha-actions.yaml` | `weather-stubs.yaml` |
| `wireguard_enabled` | `wireguard.yaml` | `wireguard-stubs.yaml` |

### Automatic Package Resolution

The config uses wrapper files to handle this. For example:
- When `airq_enabled: "true"`, ESPHome includes `airq-true.yaml`
- `airq-true.yaml` includes the actual `airq-core.yaml`
- When `airq_enabled: "false"`, ESPHome includes `airq-false.yaml`
- `airq-false.yaml` includes the actual `airq-stubs.yaml`

This is transparent to you - just change the substitution!

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
