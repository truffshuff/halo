# Device-Specific Configuration Guide

This guide explains how to create custom device configurations that include/exclude specific features and LVGL pages.

## Overview

The Halo firmware uses a modular package system where you can:
1. Enable/disable specific features (AirQ, Weather, WiFi, BLE)
2. Control which LVGL pages are included in the build
3. Optimize memory usage by only including what you need

## Package Types

### Utility Packages (Optional System Features)
- **Memory Statistics**: `memory-stats.yaml` (recommended for debugging)
- **WiFi Display**: `wifi-display.yaml` (shows WiFi connection status on display)
- **WireGuard VPN**: `wireguard.yaml` + `wireguard-display.yaml` + `time-update-wireguard.yaml`

### Feature Packages (Core + Display)
- **AirQ**: `airq-core.yaml` + `airq-display.yaml` (or `airq-stubs.yaml` to disable)
- **Weather**: `weather-core.yaml` + `weather-sensors.yaml` + `weather-fonts-text.yaml` (or `weather-stubs.yaml` to disable)
- **WiFi Monitoring**: `wifi-core.yaml` (or `wifi-stubs.yaml` to disable)
- **BLE**: `ble-core.yaml` (or `ble-stubs.yaml` to disable)

**Note:** "WiFi Display" and "WiFi Monitoring" are different:
- **WiFi Display** shows connection info (SSID, IP, signal strength) on all pages
- **WiFi Monitoring** adds a dedicated page with detailed WiFi statistics

### LVGL Page Packages
Choose ONE of these to define which pages appear on the display:

| Package | Pages Included | Use Case |
|---------|----------------|----------|
| `lvgl-pages-full.yaml` | Clock, AirQ, WiFi, Weather (all) | Full-featured device |
| `lvgl-pages-airq-weather.yaml` | Clock, AirQ, Weather | AirQ + Weather monitor |
| `lvgl-pages-airq-only.yaml` | Clock, AirQ | Air quality monitor |
| `lvgl-pages-weather.yaml` | Clock, Weather | Weather station |
| `lvgl-pages-clock-only.yaml` | Clock only | Minimal display |

## Device Configuration Examples

### Example 1: Air Quality + Weather Monitor

```yaml
# halo-living-room-airq-weather.yaml
esphome:
  name: halo-living-room
  friendly_name: Living Room AirQ + Weather
  # ... hardware config ...

packages:
  # ENABLED: AirQ monitoring
  airq_core:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/airq-core.yaml]
    refresh: always
  airq_display:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/airq-display.yaml]
    refresh: always

  # ENABLED: Weather monitoring
  weather_core:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/weather-core.yaml]
    refresh: always
  weather_sensors:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/weather-sensors.yaml]
    refresh: always
  weather_fonts_text:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/weather-fonts-text.yaml]
    refresh: always

  # DISABLED: WiFi monitoring, BLE
  wifi_stubs:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/wifi-stubs.yaml]
    refresh: always
  ble_stubs:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/ble-stubs.yaml]
    refresh: always

  # Import core config
  remote_package:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/Halo-v1-Core.yaml]
    refresh: always

  # LVGL Pages - AirQ + Weather
  lvgl_pages:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/lvgl-pages-airq-weather.yaml]
    refresh: always

# Override Default Page options
select:
  - platform: template
    name: Default Page
    id: default_page_select_boot
    entity_category: "config"
    options:
      - "Vertical Clock"
      - "AirQ"
      - "Weather"
      - "Daily Forecast"
      - "Hourly Forecast"
    initial_option: "AirQ"
    restore_value: true
    optimistic: true
    on_value:
      - lambda: |-
          int idx = 0;
          const std::string &choice = id(default_page_select_boot).state;
          if (choice == "Vertical Clock") idx = 0;
          else if (choice == "AirQ") idx = 1;
          else if (choice == "Weather") idx = 3;
          else if (choice == "Daily Forecast") idx = 4;
          else if (choice == "Hourly Forecast") idx = 5;
          id(default_page_index) = idx;

# Optimize memory - moderate buffer for AirQ + Weather
lvgl:
  buffer_size: 27%
```

### Example 2: Air Quality Monitor (AirQ Only)

```yaml
# halo-office-airq.yaml
esphome:
  name: halo-office-airq
  friendly_name: Office Air Quality Monitor
  # ... hardware config ...

packages:
  # ENABLED: AirQ monitoring
  airq_core:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/airq-core.yaml]
    refresh: always
  airq_display:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/airq-display.yaml]
    refresh: always

  # DISABLED: Weather, WiFi, BLE
  weather_stubs:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/weather-stubs.yaml]
    refresh: always
  wifi_stubs:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/wifi-stubs.yaml]
    refresh: always
  ble_stubs:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/ble-stubs.yaml]
    refresh: always

  # Import core config
  remote_package:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/Halo-v1-Core.yaml]
    refresh: always

  # LVGL Pages - AirQ only
  lvgl_pages:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/lvgl-pages-airq-only.yaml]
    refresh: always

# Override Default Page options
select:
  - platform: template
    name: Default Page
    id: default_page_select_boot
    entity_category: "config"
    options:
      - "Vertical Clock"
      - "AirQ"
    initial_option: "AirQ"
    restore_value: true
    optimistic: true
    on_value:
      - lambda: |-
          int idx = 0;
          const std::string &choice = id(default_page_select_boot).state;
          if (choice == "Vertical Clock") idx = 0;
          else if (choice == "AirQ") idx = 1;
          id(default_page_index) = idx;

# Optimize memory - higher buffer since fewer features
lvgl:
  buffer_size: 30%
```

### Example 3: Weather Station (Weather Only)

```yaml
# halo-patio-weather.yaml
esphome:
  name: halo-patio-weather
  friendly_name: Patio Weather Station
  # ... hardware config ...

packages:
  # ENABLED: Weather monitoring
  weather_core:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/weather-core.yaml]
    refresh: always
  weather_sensors:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/weather-sensors.yaml]
    refresh: always
  weather_fonts_text:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/weather-fonts-text.yaml]
    refresh: always

  # DISABLED: AirQ, WiFi monitoring, BLE
  airq_stubs:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/airq-stubs.yaml]
    refresh: always
  wifi_stubs:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/wifi-stubs.yaml]
    refresh: always
  ble_stubs:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/ble-stubs.yaml]
    refresh: always

  # Import core config
  remote_package:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/Halo-v1-Core.yaml]
    refresh: always

  # LVGL Pages - Weather only
  lvgl_pages:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/lvgl-pages-weather.yaml]
    refresh: always

# Override Default Page options
select:
  - platform: template
    name: Default Page
    id: default_page_select_boot
    entity_category: "config"
    options:
      - "Vertical Clock"
      - "Weather"
      - "Daily Forecast"
      - "Hourly Forecast"
    initial_option: "Weather"
    restore_value: true
    optimistic: true
    on_value:
      - lambda: |-
          int idx = 0;
          const std::string &choice = id(default_page_select_boot).state;
          if (choice == "Vertical Clock") idx = 0;
          else if (choice == "Weather") idx = 3;
          else if (choice == "Daily Forecast") idx = 4;
          else if (choice == "Hourly Forecast") idx = 5;
          id(default_page_index) = idx;
```

### Example 4: Full-Featured Device

```yaml
# halo-living-room-full.yaml
esphome:
  name: halo-living-room-full
  friendly_name: Living Room Halo (Full)
  # ... hardware config ...

packages:
  # ENABLED: All features
  airq_core:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/airq-core.yaml]
    refresh: always
  airq_display:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/airq-display.yaml]
    refresh: always
  weather_core:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/weather-core.yaml]
    refresh: always
  weather_sensors:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/weather-sensors.yaml]
    refresh: always
  weather_fonts_text:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/weather-fonts-text.yaml]
    refresh: always
  wifi_core:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/wifi-core.yaml]
    refresh: always

  # BLE disabled (causes memory issues)
  ble_stubs:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/ble-stubs.yaml]
    refresh: always

  # Import core config
  remote_package:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/Halo-v1-Core.yaml]
    refresh: always

  # LVGL Pages - Full configuration
  lvgl_pages:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/lvgl-pages-full.yaml]
    refresh: always

# Keep default page selection (all options available)
lvgl:
  buffer_size: 25%  # Default for full-featured device
```

### Example 5: Minimal Clock

```yaml
# halo-bedroom-clock.yaml
esphome:
  name: halo-bedroom-clock
  friendly_name: Bedroom Clock
  # ... hardware config ...

packages:
  # DISABLED: All features - stubs only
  airq_stubs:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/airq-stubs.yaml]
    refresh: always
  weather_stubs:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/weather-stubs.yaml]
    refresh: always
  wifi_stubs:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/wifi-stubs.yaml]
    refresh: always
  ble_stubs:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/ble-stubs.yaml]
    refresh: always

  # Import core config
  remote_package:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/Halo-v1-Core.yaml]
    refresh: always

  # LVGL Pages - Clock only
  lvgl_pages:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/lvgl-pages-clock-only.yaml]
    refresh: always

# No page selection needed - only one page
lvgl:
  buffer_size: 35%  # Maximum buffer for minimal config
```

## Memory Optimization Guidelines

| Configuration | Buffer Size | Notes |
|---------------|-------------|-------|
| Clock only | 30-35% | Maximum available memory |
| Clock + AirQ | 28-32% | Good balance |
| Clock + Weather | 25-28% | Weather pages need more RAM |
| Clock + AirQ + Weather | 26-28% | Both AirQ and Weather enabled |
| Full featured | 25% | All features enabled |
| With BLE enabled | 15-20% | BLE uses significant heap |

## Creating Your Own Device Config

1. **Start with a template** (one of the examples above)
2. **Choose your features** (replace core packages with stubs to disable)
3. **Select appropriate LVGL pages package** (match to your enabled features)
4. **Adjust buffer_size** based on features enabled
5. **Override Default Page select** to match available pages
6. **Test and optimize** memory usage

## Important Notes

- **Never mix core and stub packages** for the same feature (e.g., don't include both `weather_core` and `weather_stubs`)
- **LVGL pages package must match enabled features** (don't include weather pages if using weather_stubs)
- **Stub packages are required** when disabling features (they provide dummy globals that prevent compilation errors)
- **Commit and push changes to GitHub** before building in Home Assistant (it pulls from the repo)
- **Use `refresh: always`** during development to ensure latest code is pulled

## Troubleshooting

### Error: "No such file or directory"
- You're trying to include LVGL pages directly in device config (won't work with GitHub URLs)
- Solution: Use one of the `lvgl-pages-*.yaml` packages instead

### Error: "undefined reference to page_rotation_*"
- You disabled a feature but didn't include its stub package
- Solution: Include the appropriate stub package (e.g., `weather-stubs.yaml`)

### Memory/Heap Exhaustion
- Too many features enabled or buffer_size too high
- Solution: Reduce buffer_size or disable unneeded features

### Wrong pages showing
- LVGL pages package doesn't match enabled features
- Solution: Use correct `lvgl-pages-*.yaml` for your feature set
