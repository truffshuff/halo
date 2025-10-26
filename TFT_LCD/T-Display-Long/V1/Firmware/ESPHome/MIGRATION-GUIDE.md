# Migration Guide - Core Package Refactor

## What Changed

The Halo firmware architecture has been refactored to give device-specific configuration files full control over optional features.

### Before (Old Architecture)

**Halo-v1-Core.yaml** included all features by default:
- ✅ AirQ monitoring (ENABLED)
- ✅ Weather forecasting (ENABLED)
- ✅ WiFi monitoring (ENABLED)
- ✅ BLE (DISABLED via stubs)

**Device files** had to try to override these defaults by also including stubs, which could cause conflicts.

### After (New Architecture)

**Halo-v1-Core.yaml** now only includes essentials:
- ✅ Page registry
- ✅ Vertical clock
- ✅ Memory stats
- ✅ WiFi display
- ✅ WireGuard
- ✅ Time update

**Device files** now have full control:
- Each device file explicitly includes the features it wants (core packages)
- Each device file includes stubs for features it doesn't want
- No conflicts, no redundancy

## Migration Steps

### For Existing Device Configurations

If you have existing device files that import `Halo-v1-Core.yaml`, you need to update them:

1. **Add feature package includes** before the Core import
2. **Add LVGL pages package** that matches your features
3. **Update Default Page selector** to match available pages
4. **Adjust buffer_size** based on enabled features

### Example Migration

**OLD device file:**
```yaml
packages:
  remote_package:
    url: https://github.com/truffshuff/halo/
    ref: main
    files:
      - TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/Halo-v1-Core.yaml
    refresh: always
```

**NEW device file:**
```yaml
packages:
  # Enable features you want
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

  # Disable features you don't want
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

  # Import Core
  remote_package:
    url: https://github.com/truffshuff/halo/
    ref: main
    files:
      - TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/Halo-v1-Core.yaml
    refresh: always

  # Choose LVGL pages that match your features
  lvgl_pages:
    url: https://github.com/truffshuff/halo/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/lvgl-pages-airq-only.yaml]
    refresh: always
```

## Templates Available

Use these as starting points for new devices:

| Template | Features | File |
|----------|----------|------|
| AirQ Only | Clock + AirQ | `halo-v1-79e384.yaml` |
| AirQ + Weather | Clock + AirQ + Weather | `halo-v1-airq-weather-example.yaml` |

## Package Reference

### Required Feature Packages

For each feature, include EITHER core OR stubs (never both):

**AirQ:**
- Enable: `airq-core.yaml` + `airq-display.yaml`
- Disable: `airq-stubs.yaml`

**Weather:**
- Enable: `weather-core.yaml` + `weather-sensors.yaml` + `weather-fonts-text.yaml`
- Disable: `weather-stubs.yaml`

**WiFi Monitoring:**
- Enable: `wifi-core.yaml`
- Disable: `wifi-stubs.yaml`

**BLE:**
- Enable: `ble-core.yaml` (not recommended)
- Disable: `ble-stubs.yaml` (recommended)

### LVGL Pages Packages

Choose ONE that matches your enabled features:

| Package | Features |
|---------|----------|
| `lvgl-pages-full.yaml` | All (Clock, AirQ, WiFi, Weather) |
| `lvgl-pages-airq-weather.yaml` | Clock + AirQ + Weather |
| `lvgl-pages-airq-only.yaml` | Clock + AirQ |
| `lvgl-pages-weather.yaml` | Clock + Weather |
| `lvgl-pages-clock-only.yaml` | Clock only |

## Why This Is Better

1. **No Conflicts**: Feature packages only load once (from device file)
2. **Full Control**: Each device chooses exactly what it needs
3. **Memory Optimization**: Only load what you use
4. **Clear Intent**: Device config explicitly shows what features are enabled
5. **Easier Testing**: Swap features by commenting/uncommenting lines

## Troubleshooting

### Build Error: "undefined reference to page_rotation_*"
- You're missing a stub package
- Make sure EVERY feature has either core OR stubs included

### Build Error: "duplicate key 'airq_core'"
- You're including the same package twice
- Check that Core doesn't include it AND your device file doesn't include it

### Wrong pages showing
- Your LVGL pages package doesn't match enabled features
- Use `lvgl-pages-airq-only.yaml` if you only have AirQ enabled
- Use `lvgl-pages-full.yaml` if all features are enabled

### Memory/Heap Issues
- Reduce `buffer_size` in LVGL section
- Disable features you don't need
- See memory guidelines in README-DEVICE-CONFIGURATION.md

## Further Reading

- [README-DEVICE-CONFIGURATION.md](packages/README-DEVICE-CONFIGURATION.md) - Complete device configuration guide with examples
- [README-AIRQ-INTEGRATION.md](packages/README-AIRQ-INTEGRATION.md) - AirQ package details
- [README-WEATHER-INTEGRATION.md](packages/README-WEATHER-INTEGRATION.md) - Weather package details (if it exists)
- [README-WIFI-INTEGRATION.md](packages/README-WIFI-INTEGRATION.md) - WiFi package details
- [README-BLE-INTEGRATION.md](packages/README-BLE-INTEGRATION.md) - BLE package details
