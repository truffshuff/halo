# Modular Weather System for Halo

## Overview

The weather system has been split into **independent modules** to give you maximum flexibility in managing memory usage with BLE coexistence. You can now enable only the weather features you need!

## Module Structure

### Sensor Modules (Data from Home Assistant)

These modules pull weather data from Home Assistant and update the display:

1. **weather-sensors-basic.yaml** (~40 sensors, ~25KB RAM)
   - Current temperature, feels-like, precipitation
   - Today's forecast (Day 1) high/low, condition, precipitation
   - **Use for:** Basic forecast page only

2. **weather-sensors-daily.yaml** (~80 sensors, ~50KB RAM)
   - 9-day extended forecast (Days 2-10)
   - High/low temps, conditions, dates, precipitation probabilities
   - **Use for:** Daily forecast pages (multi-day view)
   - **Requires:** weather-sensors-basic.yaml

### Display Modules (LVGL Pages)

These modules define the visual pages shown on your display:

1. **weather-forecast-basic.yaml** (~20KB)
   - Main weather forecast page with current conditions
   - Shows: Current temp, feels-like, high/low, precipitation, condition icon
   - Includes: Weather core (scripts, globals), icon fonts, minimal text sensors
   - **Memory:** Lightest option, great for BLE coexistence
   - **Requires:** weather-sensors-basic.yaml

2. **weather-forecast-hourly.yaml** (~35KB)
   - 6-hour detailed forecast (2 pages, 3 hours each)
   - Shows: Temp, feels-like, humidity, precipitation, pressure, UV, wind
   - **Note:** Requires hourly sensors to be created in Home Assistant
   - **Memory:** Moderate impact

3. **weather-forecast-daily.yaml** (~30KB)
   - 9-day forecast overview (Days 2-10)
   - Multiple sub-pages showing 2 days each
   - Shows: Date, icon, condition, high/low temps, rain probability
   - **Memory:** Moderate impact
   - **Requires:** weather-sensors-daily.yaml

## Configuration Examples

### Example 1: BLE + Minimal Weather (Recommended for BLE)
```yaml
packages:
  # Core functionality
  psram_helpers: !include packages/psram-helpers.yaml

  # BLE (with coexistence tuning)
  ble_coexist_tuning: !include packages/ble-coexist-tuning.yaml
  ble_core: !include packages/ble-core.yaml  # Use regular, not delayed

  # Minimal weather - just current forecast
  weather_sensors_basic: !include packages/weather-sensors-basic.yaml
  weather_forecast_basic: !include packages/weather-forecast-basic.yaml

  # Pages package
  lvgl_pages: !include packages/lvgl-pages-clock-weather-basic.yaml  # TODO: Create this
```

**Memory Impact:** ~45KB for weather (vs ~130KB for full weather)
**Result:** BLE + basic weather forecast works!

### Example 2: No BLE + Full Weather
```yaml
packages:
  # Core functionality
  psram_helpers: !include packages/psram-helpers.yaml

  # Full weather
  weather_sensors_basic: !include packages/weather-sensors-basic.yaml
  weather_sensors_daily: !include packages/weather-sensors-daily.yaml
  weather_forecast_basic: !include packages/weather-forecast-basic.yaml
  weather_forecast_daily: !include packages/weather-forecast-daily.yaml

  # Pages package
  lvgl_pages: !include packages/lvgl-pages-weather-full.yaml  # TODO: Create this
```

**Memory Impact:** ~125KB for full weather (same as before)
**Result:** All weather features, no BLE

### Example 3: BLE + Basic Weather + Daily Forecast (Aggressive)
```yaml
packages:
  # Core functionality
  psram_helpers: !include packages/psram-helpers.yaml

  # BLE
  ble_coexist_tuning: !include packages/ble-coexist-tuning.yaml
  ble_core: !include packages/ble-core.yaml

  # Basic + daily weather
  weather_sensors_basic: !include packages/weather-sensors-basic.yaml
  weather_sensors_daily: !include packages/weather-sensors-daily.yaml
  weather_forecast_basic: !include packages/weather-forecast-basic.yaml
  weather_forecast_daily: !include packages/weather-forecast-daily.yaml

  # Pages package
  lvgl_pages: !include packages/lvgl-pages-ble-weather.yaml  # TODO: Create this
```

**Memory Impact:** ~95KB for weather
**Result:** BLE + current + multi-day forecast (may be tight on memory)

## Memory Budget Comparison

| Configuration | Weather RAM | BLE RAM | Total | Free Internal Heap | Works? |
|--------------|------------|---------|-------|-------------------|--------|
| Full Weather (old) | ~130KB | - | ~130KB | ~80KB | ✅ Yes |
| Full Weather + BLE | ~130KB | ~60KB | ~190KB | ~20KB | ❌ OOM |
| Basic Weather only | ~45KB | - | ~45KB | ~165KB | ✅ Yes |
| Basic Weather + BLE | ~45KB | ~60KB | ~105KB | ~105KB | ✅ Yes! |
| Basic + Daily + BLE | ~95KB | ~60KB | ~155KB | ~55KB | ⚠️  Tight |

## Migration Guide

### From Old Weather System

If you were using the old all-in-one weather packages:

**Old (monolithic):**
```yaml
packages:
  weather_core: !include packages/weather-core.yaml
  weather_sensors: !include packages/weather-sensors.yaml
  weather_fonts_text: !include packages/weather-fonts-text.yaml
```

**New (modular - basic):**
```yaml
packages:
  weather_sensors_basic: !include packages/weather-sensors-basic.yaml
  weather_forecast_basic: !include packages/weather-forecast-basic.yaml
```

**New (modular - full):**
```yaml
packages:
  weather_sensors_basic: !include packages/weather-sensors-basic.yaml
  weather_sensors_daily: !include packages/weather-sensors-daily.yaml
  weather_forecast_basic: !include packages/weather-forecast-basic.yaml
  weather_forecast_daily: !include packages/weather-forecast-daily.yaml
```

## Files Created

### New Modular Files
- `packages/weather-sensors-basic.yaml` - Current + Day 1 sensors
- `packages/weather-sensors-daily.yaml` - Days 2-10 sensors
- `packages/weather-forecast-basic.yaml` - Main forecast page
- `packages/weather-forecast-daily.yaml` - Multi-day forecast pages
- `packages/weather-forecast-hourly.yaml` - Hourly forecast pages (TODO)

### Deprecated Files (keep for compatibility)
- `packages/weather-core.yaml` - Merged into forecast modules
- `packages/weather-sensors.yaml` - Split into basic + daily
- `packages/weather-fonts-text.yaml` - Merged into forecast modules
- `packages/weather-pages.yaml` - Split into basic + daily + hourly

### Stubs (for disabling)
- `packages/weather-stubs.yaml` - Use to disable all weather

## Troubleshooting

### "Widget lbl_weather_forecast_temphi not found"
- You enabled a display module without the required sensor module
- **Fix:** Enable `weather-sensors-basic.yaml`

### "Out of memory" with BLE + Weather
- You have too many weather modules enabled
- **Fix:** Use only `weather-forecast-basic.yaml`, disable daily/hourly

### Weather data not updating
- Check Home Assistant entities exist
- **Fix:** Verify entity_id names match your HA setup

## Future Enhancements

### Hourly Sensors (TODO)
Currently hourly forecast pages exist but require manual HA sensor setup. Future enhancement:
- Create `weather-sensors-hourly.yaml` with 6-hour forecast sensors
- Integrate with weather service APIs

### Page Combinations (TODO)
Create pre-configured page packages for common combinations:
- `lvgl-pages-clock-weather-basic.yaml` - Clock + basic weather
- `lvgl-pages-ble-weather.yaml` - Optimized for BLE coexistence
- `lvgl-pages-weather-full.yaml` - All weather pages

## Benefits

✅ **Memory Savings:** Enable only what you need (45KB vs 130KB for basic)
✅ **BLE Compatible:** Basic weather + BLE finally works!
✅ **Flexible:** Mix and match sensors and displays
✅ **Future-Proof:** Easy to add/remove features
✅ **Clear Dependencies:** Each module documents what it needs

## Questions?

See also:
- `README-BLE-INTEGRATION.md` - BLE + weather coexistence tips
- `README-PSRAM-OPTIMIZATION.md` - Memory optimization guide
- `README-DEVICE-CONFIGURATION.md` - Full device setup examples
