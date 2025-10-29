# Weather-Forecast-Basic Package - Known Issue

## Problem

The `weather-forecast-basic.yaml` package was created with a design flaw. It attempted to include LVGL page definitions inside a package file, which doesn't work properly with ESPHome's package system.

## Error Messages You'll See

```
lvgl: [source /data/packages/.../weather-forecast-basic.yaml:272]
  - pages:

Couldn't find ID 'icons_80'
```

## Root Cause

ESPHome packages cannot contain top-level `lvgl:` sections. LVGL pages must be defined in the main configuration file or in a dedicated pages package.

## Current Status

The `weather-forecast-basic.yaml` file has been updated to:
- ✅ Include weather core functionality (globals, scripts, switches)
- ✅ Include all icon fonts (icons_20, icons_40, icons_50, icons_80, icons_100)
- ✅ Include LVGL text sensors for weather widgets
- ❌ NO longer includes page definitions (those belong elsewhere)

## Working Solutions

### SOLUTION 1: Use Existing Full Weather (Recommended for Now)

Until custom minimal pages are created, use the existing weather system:

```yaml
packages:
  # PSRAM helpers (required)
  psram_helpers:
    url: https://github.com/truffshuff/halo.git/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/psram-helpers.yaml]
    refresh: always

  # Full weather (works now)
  weather_core:
    url: https://github.com/truffshuff/halo.git/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/weather-core.yaml]
    refresh: always
  weather_sensors:
    url: https://github.com/truffshuff/halo.git/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/weather-sensors.yaml]
    refresh: always
  weather_fonts_text:
    url: https://github.com/truffshuff/halo.git/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/weather-fonts-text.yaml]
    refresh: always

  # Use existing pages package
  lvgl_pages:
    url: https://github.com/truffshuff/halo.git/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/lvgl-pages-full-wireguard.yaml]
    refresh: always

# Important: Set buffer based on whether BLE is enabled
lvgl:
  buffer_size: 25%  # If NO BLE
  # buffer_size: 15%  # If BLE enabled (may be tight with full weather)
```

### SOLUTION 2: Disable Weather (For BLE Priority)

If you want BLE and don't mind losing weather:

```yaml
packages:
  # BLE
  ble_coexist_tuning: !include ...
  ble_core_simplified: !include ...  # or ble-core.yaml

  # NO weather
  weather_stubs: !include ...

  # Pages without weather
  lvgl_pages: !include lvgl-pages-airq-wifi.yaml

lvgl:
  buffer_size: 20%  # More room without weather
```

### SOLUTION 3: Future - Custom Minimal Pages Package (Not Yet Available)

Eventually we'll create `lvgl-pages-airq-weather-basic.yaml` which will:
- Include clock, AirQ, WiFi, main weather pages
- Exclude hourly and daily forecast pages
- Work with `weather-forecast-basic.yaml`

This package doesn't exist yet and needs to be manually created.

## What Changed

### weather-forecast-basic.yaml (Fixed)
```yaml
# NOW INCLUDES:
- Weather core (globals, scripts)
- All 5 icon font sizes (20, 40, 50, 80, 100)
- LVGL text sensors for weather
- Weather gauge controls

# NO LONGER INCLUDES:
- LVGL page definitions (moved to lvgl-pages packages)
```

## Memory Impact Reality

Even with the modular approach, the memory math is challenging:

| Component | Memory |
|-----------|--------|
| Base + LVGL 15% | ~55KB |
| AirQ | ~20KB |
| WiFi Monitoring | ~5KB |
| Weather Sensors (all) | ~75KB |
| Weather Core + Fonts | ~25KB |
| BLE | ~60KB |
| **Total** | **~240KB** |
| **Available** | **~210KB** |
| **Deficit** | **-30KB** ❌ |

**Conclusion:** Even with "basic" weather, if you include all sensors from `weather-sensors.yaml`, it's still too much with BLE.

## True Minimal Weather for BLE

To make BLE + Weather work, you'd need to:

1. **Create minimal sensors file** (~10-15 sensors only):
   - Current temp
   - Current condition
   - Today's high/low
   - Precipitation
   - Skip all 10-day forecast data

2. **Create minimal page**:
   - Just current weather display
   - No multi-day forecasts

3. **Memory saved**:
   - Minimal sensors: ~10KB (vs ~75KB)
   - Total with BLE: ~150KB (vs ~240KB)
   - **Free RAM: ~60KB** ✅ Works!

## Recommendations

### For Your Current Situation

**If you want BLE:**
```yaml
# Choose ONE:
# A) BLE + No Weather (safest)
# B) BLE + Disable some other features
# C) Wait for true minimal weather sensors file
```

**If you want Full Weather:**
```yaml
# Disable BLE
# Use existing weather-core + weather-sensors + weather-fonts-text
# Use lvgl-pages-full.yaml
# Set buffer_size: 25%
```

### Short Term Fix

Comment out `weather-forecast-basic` in your config and use the existing full weather packages (weather_core, weather_sensors, weather_fonts_text) with an existing pages package.

### Long Term Solution

Someone needs to create:
1. `weather-sensors-minimal.yaml` - Just ~10 sensors for current weather
2. `lvgl-pages-ble-weather.yaml` - Pages optimized for BLE + minimal weather

## Apology

The `weather-forecast-basic.yaml` package was created with good intentions but had architectural issues:
- Tried to include pages in a package (doesn't work)
- Still relied on full `weather-sensors.yaml` (too many sensors)
- Didn't actually save enough memory for BLE

The real solution requires creating:
- A truly minimal sensors file
- Custom pages package
- Proper integration

This is more work than initially anticipated.

## Quick Decision Guide

**Right Now:**
1. Want BLE? → Disable weather, use `ble-core-simplified.yaml`
2. Want Weather? → Disable BLE, use existing weather packages
3. Want Both? → Pick which is more important, or wait for proper minimal weather

**Future:**
- Create `weather-sensors-minimal.yaml` with ~10 sensors
- Create `lvgl-pages-ble-weather.yaml` with minimal pages
- Test memory with that combination

## Files Status

| File | Status | Notes |
|------|--------|-------|
| weather-forecast-basic.yaml | ⚠️ Incomplete | Core/fonts work, but pages removed |
| weather-sensors.yaml | ✅ Works | But too big for BLE (~75KB) |
| ble-core-simplified.yaml | ✅ Works | Simplified BLE |
| lvgl-pages-airq-weather-basic.yaml | ❌ Missing | Needs to be created |
| weather-sensors-minimal.yaml | ❌ Missing | Needs to be created |

Sorry for the confusion. The modular approach is the right direction, but needs more work to be truly functional for BLE + Weather coexistence.
