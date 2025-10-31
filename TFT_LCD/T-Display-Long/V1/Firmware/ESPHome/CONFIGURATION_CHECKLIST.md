# Halo V1 Configuration Checklist

Use this checklist when setting up your device configuration. Print it out or keep it open while editing `halo-v1-79e384.yaml`.

## Step 1: Choose Your Features

Decide which features you want, then fill in the substitutions section (lines 43-66):

- [ ] Air Quality Monitoring (AirQ sensors)
  - If YES: set `airq_enabled: "true"`
  - If NO: set `airq_enabled: "false"`

- [ ] Bluetooth Presence Detection (BLE)
  - If YES: set `ble_enabled: "true"`
  - If NO: set `ble_enabled: "false"`

- [ ] WiFi Status Display
  - If YES: set `wifi_enabled: "true"`
  - If NO: set `wifi_enabled: "false"`

- [ ] Weather Forecast
  - If YES: set `weather_enabled: "true"`
  - If NO: set `weather_enabled: "false"`

- [ ] WireGuard VPN (requires WiFi enabled)
  - If YES: set `wireguard_enabled: "true"` AND `wifi_enabled: "true"`
  - If NO: set `wireguard_enabled: "false"`

## Step 2: Select LVGL Pages Combo (Line 82)

Based on what you enabled above, select ONE:

```
If ONLY WiFi enabled:
  lvgl_pages_combo: "wifi-only"

If ONLY AirQ enabled:
  lvgl_pages_combo: "airq-only"

If ONLY Weather enabled:
  lvgl_pages_combo: "weather-only"

If WiFi + WireGuard (no AirQ):
  lvgl_pages_combo: "wireguard-only"

If AirQ + WiFi:
  lvgl_pages_combo: "airq-wifi"

If AirQ + Weather:
  lvgl_pages_combo: "airq-weather"

If WiFi + Weather:
  lvgl_pages_combo: "wifi-weather"

If AirQ + WiFi + WireGuard:
  lvgl_pages_combo: "airq-wifi-wireguard"

If AirQ + WiFi + Weather:
  lvgl_pages_combo: "airq-wifi-weather"

If AirQ + WiFi + Weather + WireGuard:
  lvgl_pages_combo: "airq-wifi-weather-wg"

If Nothing enabled (clock only):
  lvgl_pages_combo: "clock-only"
```

## Step 3: Select Time Update Package (Line 117)

Comment out all except ONE based on your features:

```
# For clock-only:
# time_update_script: !include packages/time-update-clock-only.yaml

# For WiFi-only (airq_enabled="false", wifi_enabled="true"):
time_update_script: !include packages/time-update-wifi-only.yaml

# For WiFi + WireGuard (no AirQ):
# time_update_script: !include packages/time-update-wireguard-only.yaml

# For AirQ or any mix with AirQ (but NO WireGuard):
# time_update_script: !include packages/time-update-basic.yaml

# For AirQ + WiFi + WireGuard:
# time_update_script: !include packages/time-update-wireguard.yaml
```

## Step 4: Configure Feature Packages (Lines 113-143)

For EACH feature, ensure ONLY ONE line is uncommented:

### AirQ (Lines 113-115)

```
If airq_enabled: "true"
✓ airq: !include packages/airq-core.yaml
✗ # airq: !include packages/airq-stubs.yaml

If airq_enabled: "false"
✗ # airq: !include packages/airq-core.yaml
✓ airq: !include packages/airq-stubs.yaml
```

### BLE (Lines 117-119)

```
If ble_enabled: "true"
✓ ble: !include packages/ble-core-simplified.yaml
✗ # ble: !include packages/ble-stubs.yaml

If ble_enabled: "false"
✗ # ble: !include packages/ble-core-simplified.yaml
✓ ble: !include packages/ble-stubs.yaml
```

### WiFi (Lines 121-123)

```
If wifi_enabled: "true"
✓ wifi_core: !include packages/wifi-core.yaml
✓ wifi_display: !include packages/wifi-display.yaml
✗ # wifi: !include packages/wifi-stubs.yaml

If wifi_enabled: "false"
✗ # wifi_core: !include packages/wifi-core.yaml
✗ # wifi_display: !include packages/wifi-display.yaml
✓ wifi: !include packages/wifi-stubs.yaml
```

### Weather (Lines 125-127)

```
If weather_enabled: "true"
✓ weather: !include packages/weather-ha-actions.yaml
✗ # weather: !include packages/weather-stubs.yaml

If weather_enabled: "false"
✗ # weather: !include packages/weather-ha-actions.yaml
✓ weather: !include packages/weather-stubs.yaml
```

### WireGuard (Lines 129-131)

⚠️ **NOTE:** Only set to true if BOTH wireguard_enabled AND wifi_enabled are "true"

```
If wireguard_enabled: "true" AND wifi_enabled: "true"
✓ wireguard: !include packages/wireguard.yaml
✗ # wireguard: !include packages/wireguard-stubs.yaml

Otherwise:
✗ # wireguard: !include packages/wireguard.yaml
✓ wireguard: !include packages/wireguard-stubs.yaml
```

## Step 5: Select LVGL Pages Package (Lines 143-150)

Comment out all except ONE that matches your `lvgl_pages_combo`:

```
If lvgl_pages_combo: "clock-only"
✓ lvgl_pages: !include packages/lvgl-pages-clock-only.yaml

If lvgl_pages_combo: "airq-only"
✓ lvgl_pages: !include packages/lvgl-pages-airq-only.yaml

If lvgl_pages_combo: "wifi-only"
✓ lvgl_pages: !include packages/lvgl-pages-wifi-only.yaml

If lvgl_pages_combo: "weather-only"
✓ lvgl_pages: !include packages/lvgl-pages-weather-only.yaml

If lvgl_pages_combo: "airq-wifi"
✓ lvgl_pages: !include packages/lvgl-pages-airq-wifi.yaml

If lvgl_pages_combo: "airq-weather"
✓ lvgl_pages: !include packages/lvgl-pages-airq-weather-forecast-only.yaml

If lvgl_pages_combo: "wifi-weather"
✓ lvgl_pages: !include packages/lvgl-pages-wifi-weather-forecast-only.yaml

If lvgl_pages_combo: "airq-wifi-weather"
✓ lvgl_pages: !include packages/lvgl-pages-airq-wifi-weather-forecast.yaml
```

## ⚠️ SPECIAL CASES

### If you enable WiFi + AirQ later:

In `wifi-display.yaml`, uncomment these sections to show WiFi indicator on AirQ page:
- Lines 105-117 (WiFi connected case)
- Lines 204-216 (WiFi disconnected case)

### If you enable WireGuard:

**CRITICAL:** You MUST also enable AirQ!
- Set `airq_enabled: "true"`
- Include `airq-core.yaml` and matching LVGL pages

## Verification Checklist

Before compiling, verify:

- [ ] Feature toggles (lines 43-66) are set correctly
- [ ] lvgl_pages_combo (line 82) matches your enabled features
- [ ] time_update_script (line 117) is correct for your feature combo
- [ ] For EACH feature:
  - [ ] If enabled: -core.yaml is uncommented, -stubs.yaml is commented
  - [ ] If disabled: -stubs.yaml is uncommented, -core.yaml is commented
- [ ] LVGL pages package (line 143+) matches lvgl_pages_combo
- [ ] If AirQ+WiFi: Remember to uncomment ym_image sections in wifi-display.yaml later

## Quick Reference

| Your Setup | time_update | lvgl_pages | airq | wifi | weather | wireguard |
|---|---|---|---|---|---|---|
| Clock only | clock-only | clock-only | stubs | stubs | stubs | stubs |
| WiFi only | wifi-only | wifi-only | stubs | core | stubs | stubs |
| AirQ only | basic | airq-only | core | stubs | stubs | stubs |
| Weather only | basic | weather-only | stubs | stubs | core | stubs |
| WiFi+WireGuard | wireguard-only | wireguard-only | stubs | core | stubs | core |
| AirQ+WiFi | basic | airq-wifi | core | core | stubs | stubs |
| AirQ+Weather | basic | airq-weather | core | stubs | core | stubs |
| WiFi+Weather | basic | wifi-weather | stubs | core | core | stubs |
| AirQ+WiFi+WireGuard | wireguard | airq-wifi | core | core | stubs | core |
| AirQ+WiFi+Weather | basic | airq-wifi-weather | core | core | core | stubs |

## Need Help?

See `PACKAGE_SELECTION_GUIDE.md` for detailed explanations of each setting.
