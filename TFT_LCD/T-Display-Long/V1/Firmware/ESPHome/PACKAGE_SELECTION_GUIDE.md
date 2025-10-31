# Halo V1 ESPHome Package Selection Guide

This guide explains how to properly configure your device by selecting the correct packages based on which features you want to enable.

## Quick Start

1. **Set your feature toggles** in `halo-v1-79e384.yaml` (lines 43-66)
2. **Select the matching `lvgl_pages_combo`** (line 82)
3. **Update package includes** (lines 90-143) to match your features
4. **Select the correct time-update package** (line 117) based on the chart below

## Feature-to-Package Mapping

### Step 1: Feature Toggles (Lines 43-66)

Set these to "true" or "false" based on what you want:

```yaml
airq_enabled: "false"          # Air Quality sensors
ble_enabled: "false"           # Bluetooth presence detection
wifi_enabled: "true"           # WiFi status page
weather_enabled: "false"       # Weather forecast
wireguard_enabled: "false"     # VPN tunnel status
```

### Step 2: LVGL Pages Combo (Line 82)

Choose the matching `lvgl_pages_combo`:

| Your Features | lvgl_pages_combo | Notes |
|---|---|---|
| None (Clock only) | `clock-only` | Minimal, BLE compatible |
| AirQ only | `airq-only` | Air quality monitoring |
| WiFi only | `wifi-only` | Network status only |
| Weather only | `weather-only` | Forecast display |
| AirQ + WiFi | `airq-wifi` | Popular combination |
| AirQ + Weather | `airq-weather` | Air quality + forecast |
| WiFi + Weather | `wifi-weather` | Network + forecast |
| AirQ + WiFi + Weather | `airq-wifi-weather` | All features |

### Step 3: Time Update Package (Line 117)

Choose based on your enabled features:

| Your Enabled Features | Package to Use | Why |
|---|---|---|
| Clock only | `time-update-clock-only.yaml` | Only updates vertical clock |
| WiFi + Clock (NO AirQ) | `time-update-wifi-only.yaml` | Only updates vertical clock |
| WiFi + WireGuard (NO AirQ) | `time-update-wireguard-only.yaml` | Clock + SNTP/HA time switching, no AirQ |
| AirQ + any other feature (NO WireGuard) | `time-update-basic.yaml` | Updates clock + AirQ page time |
| AirQ + WiFi + WireGuard | `time-update-wireguard.yaml` | Clock + AirQ + SNTP/HA time switching |

### Step 4: Feature Packages (Lines 113-143)

For each feature, uncomment the **-core.yaml** line and comment out the **-stubs.yaml** line:

#### Air Quality (Lines 113-115)

```yaml
# If airq_enabled = "true"
airq: !include packages/airq-core.yaml
# airq: !include packages/airq-stubs.yaml

# If airq_enabled = "false"
# airq: !include packages/airq-core.yaml
airq: !include packages/airq-stubs.yaml
```

#### BLE (Lines 117-119)

```yaml
# If ble_enabled = "true"
ble: !include packages/ble-core-simplified.yaml
# ble: !include packages/ble-stubs.yaml

# If ble_enabled = "false"
# ble: !include packages/ble-core-simplified.yaml
ble: !include packages/ble-stubs.yaml
```

#### WiFi (Lines 121-123)

```yaml
# If wifi_enabled = "true"
wifi_core: !include packages/wifi-core.yaml
wifi_display: !include packages/wifi-display.yaml
# wifi: !include packages/wifi-stubs.yaml

# If wifi_enabled = "false"
# wifi_core: !include packages/wifi-core.yaml
# wifi_display: !include packages/wifi-display.yaml
wifi: !include packages/wifi-stubs.yaml
```

#### Weather (Lines 125-127)

```yaml
# If weather_enabled = "true"
weather: !include packages/weather-ha-actions.yaml
# weather: !include packages/weather-stubs.yaml

# If weather_enabled = "false"
# weather: !include packages/weather-ha-actions.yaml
weather: !include packages/weather-stubs.yaml
```

#### WireGuard (Lines 129-131)

```yaml
# If wireguard_enabled = "true" (requires wifi_enabled = "true")
wireguard: !include packages/wireguard.yaml
# wireguard: !include packages/wireguard-stubs.yaml

# If wireguard_enabled = "false"
# wireguard: !include packages/wireguard.yaml
wireguard: !include packages/wireguard-stubs.yaml
```

### Step 5: LVGL Pages Package (Lines 143-150)

Uncomment only ONE line that matches your `lvgl_pages_combo`:

```yaml
# lvgl_pages: !include packages/lvgl-pages-airq-only.yaml
# lvgl_pages: !include packages/lvgl-pages-clock-only.yaml
lvgl_pages: !include packages/lvgl-pages-wifi-only.yaml
# lvgl_pages: !include packages/lvgl-pages-weather-only.yaml
# lvgl_pages: !include packages/lvgl-pages-airq-wifi.yaml
# lvgl_pages: !include packages/lvgl-pages-airq-weather-forecast-only.yaml
# lvgl_pages: !include packages/lvgl-pages-wifi-weather-forecast-only.yaml
# lvgl_pages: !include packages/lvgl-pages-airq-wifi-weather-forecast.yaml
```

## Complete Configuration Examples

### Example 1: WiFi-Only (Current Configuration)

```yaml
# Feature Toggles
airq_enabled: "false"
ble_enabled: "false"
wifi_enabled: "true"         # ← ENABLED
weather_enabled: "false"
wireguard_enabled: "false"

# LVGL Pages
lvgl_pages_combo: "wifi-only"

# Time Update
time_update_script: !include packages/time-update-wifi-only.yaml

# Feature Packages
airq: !include packages/airq-stubs.yaml
ble: !include packages/ble-stubs.yaml
wifi_core: !include packages/wifi-core.yaml      # ← ENABLED
wifi_display: !include packages/wifi-display.yaml # ← ENABLED
weather: !include packages/weather-stubs.yaml
wireguard: !include packages/wireguard-stubs.yaml

# LVGL Pages
lvgl_pages: !include packages/lvgl-pages-wifi-only.yaml
```

### Example 2: AirQ + WiFi

```yaml
# Feature Toggles
airq_enabled: "true"         # ← ENABLED
ble_enabled: "false"
wifi_enabled: "true"         # ← ENABLED
weather_enabled: "false"
wireguard_enabled: "false"

# LVGL Pages
lvgl_pages_combo: "airq-wifi"

# Time Update
time_update_script: !include packages/time-update-basic.yaml

# Feature Packages
airq: !include packages/airq-core.yaml           # ← ENABLED
ble: !include packages/ble-stubs.yaml
wifi_core: !include packages/wifi-core.yaml      # ← ENABLED
wifi_display: !include packages/wifi-display.yaml # ← ENABLED
weather: !include packages/weather-stubs.yaml
wireguard: !include packages/wireguard-stubs.yaml

# LVGL Pages
lvgl_pages: !include packages/lvgl-pages-airq-wifi.yaml

# IMPORTANT: When AirQ is enabled, uncomment the ym_image show/hide
# sections in wifi-display.yaml (lines 105-117 and 204-216) to show
# the WiFi indicator on the AirQ page header
```

### Example 3: AirQ + WiFi + WireGuard

```yaml
# Feature Toggles
airq_enabled: "true"         # ← ENABLED
ble_enabled: "false"
wifi_enabled: "true"         # ← ENABLED
weather_enabled: "false"
wireguard_enabled: "true"    # ← ENABLED

# LVGL Pages
lvgl_pages_combo: "airq-wifi"

# Time Update
time_update_script: !include packages/time-update-wireguard.yaml

# Feature Packages
airq: !include packages/airq-core.yaml           # ← ENABLED
ble: !include packages/ble-stubs.yaml
wifi_core: !include packages/wifi-core.yaml      # ← ENABLED
wifi_display: !include packages/wifi-display.yaml # ← ENABLED
weather: !include packages/weather-stubs.yaml
wireguard: !include packages/wireguard.yaml      # ← ENABLED

# LVGL Pages
lvgl_pages: !include packages/lvgl-pages-airq-wifi-wireguard.yaml
```

## Important Notes

### About time-update-basic.yaml

When using `time-update-basic.yaml` with AirQ enabled, the AirQ time update section is currently commented out in the file (lines 49-57). You have two options:

**Option A (Recommended):** Use the already-uncommented version for your specific configuration
- `time-update-wifi-only.yaml` - for WiFi + Clock
- Create `time-update-airq.yaml` - for AirQ + Clock

**Option B:** Uncomment it manually in time-update-basic.yaml:
```yaml
# Uncomment these lines if airq_enabled = "true"
- if:
    condition:
      lambda: 'return id(time_update_needs_render) && id(current_page_index) == 1;'
    then:
      - lvgl.label.update:
          id: timeVal
          text: !lambda 'return id(time_update_last_text);'
```

### About WireGuard Requirements

⚠️ **CRITICAL:** WireGuard mode REQUIRES AirQ to be enabled:
- `time-update-wireguard.yaml` always tries to update the `timeVal` label on the AirQ page
- If you disable AirQ but enable WireGuard, the firmware will compile but crash at runtime
- Always set `airq_enabled: "true"` when using WireGuard

### About WiFi Page and AirQ

When AirQ is enabled alongside WiFi:
- The `wifi-display.yaml` package has special handling for the `ym_image` (WiFi logo)
- In wifi-only mode: these sections are commented out (no AirQ page to show the logo on)
- In airq+wifi mode: uncomment lines 105-117 and 204-216 in `wifi-display.yaml` to enable the WiFi indicator on the AirQ page header

## Troubleshooting

### Error: "Couldn't find ID 'xyz'"

This happens when a package tries to reference a widget that doesn't exist.

**Solution:** Check that:
1. The feature is enabled in substitutions
2. The matching feature package is set to `-core.yaml` (not `-stubs.yaml`)
3. The LVGL pages package includes the required page
4. The time-update package is correct for your feature combination

### Error: "Couldn't find ID 'timeVal'"

This happens when using `time-update-basic.yaml` without AirQ enabled.

**Solution:**
- If you want WiFi-only: use `time-update-wifi-only.yaml` instead
- If you want AirQ: set `airq_enabled: "true"` and include `airq-core.yaml`

### Error: "Couldn't find ID 'wifi_stat'"

This happens when AirQ is disabled but `airq-core.yaml` is still included.

**Solution:** Set `airq: !include packages/airq-stubs.yaml` when `airq_enabled = "false"`

### Compilation succeeds but WiFi page doesn't update

**Possible cause:** You're using the wrong time-update package or have mismatched feature selections.

**Solution:**
1. Verify `lvgl_pages_combo` matches your enabled features
2. Verify the time-update package matches the table above
3. Check that all `-core.yaml` packages are included for enabled features
4. Check that all `-stubs.yaml` packages are included for disabled features

## Supported Feature Combinations

| Config | Features | Status | Notes |
|--------|----------|--------|-------|
| Clock only | None | ✅ Stable | Minimal, BLE compatible |
| WiFi only | WiFi | ✅ Current | Network monitoring only |
| AirQ only | AirQ | ✅ Stable | Air quality monitoring |
| Weather only | Weather | ✅ Stable | Forecast display |
| WiFi + WireGuard | WiFi + VPN | ✅ Stable | Network + VPN tunnel |
| AirQ + WiFi | AirQ + WiFi | ✅ Stable | Popular combo |
| AirQ + Weather | AirQ + Weather | ✅ Stable | Air quality + forecast |
| WiFi + Weather | WiFi + Weather | ✅ Stable | Network + weather |
| AirQ + WiFi + Weather | All three | ✅ Stable | Maximum features |
| AirQ + WiFi + WireGuard | All plus VPN | ✅ Stable | Full features + VPN |

## Summary

The key is matching:
1. **Feature toggles** ↔ Enabled features
2. **lvgl_pages_combo** ↔ Feature combination
3. **Feature packages** ↔ Enable -core.yaml for enabled features, -stubs.yaml for disabled
4. **time_update_script** ↔ Your specific feature combination
5. **lvgl_pages package** ↔ Matches lvgl_pages_combo

When everything aligns, the device compiles and works correctly!
