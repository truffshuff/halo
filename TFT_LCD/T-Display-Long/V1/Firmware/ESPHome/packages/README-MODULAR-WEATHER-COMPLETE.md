# Modular Weather System - Complete Guide

## Overview

The weather system has been completely modularized into separate sensor and page files! You can now mix and match components to fit your memory budget.

## File Structure

### Sensor Files (Data from Home Assistant)

| File | Sensors | RAM | Compatible with BLE? |
|------|---------|-----|---------------------|
| [weather-sensors-current.yaml](weather-sensors-current.yaml) | 8 | ~5KB | ✅ YES |
| [weather-sensors-daily.yaml](weather-sensors-daily.yaml) | 45 | ~30KB | ⚠️ Maybe (tight) |
| [weather-sensors-hourly.yaml](weather-sensors-hourly.yaml) | 72 | ~45KB | ❌ NO |

### Page Files (LVGL Display Pages)

| File | Description | Pages |
|------|-------------|-------|
| [weather-page-forecast.yaml](weather-page-forecast.yaml) | Main forecast page | 1 (current weather) |
| [weather-page-daily.yaml](weather-page-daily.yaml) | Daily page (Days 2-3) | 1 |
| [weather-page-days-4-5.yaml](weather-page-days-4-5.yaml) | Daily page (Days 4-5) | 1 |
| [weather-page-days-6-7.yaml](weather-page-days-6-7.yaml) | Daily page (Days 6-7) | 1 |
| [weather-page-days-8-9.yaml](weather-page-days-8-9.yaml) | Daily page (Days 8-9) | 1 |
| [weather-page-day-10.yaml](weather-page-day-10.yaml) | Daily page (Day 10) | 1 |
| [weather-page-hourly.yaml](weather-page-hourly.yaml) | Hourly page (Hours 1-3) | 1 |
| [weather-page-hourly-2.yaml](weather-page-hourly-2.yaml) | Hourly page (Hours 4-6) | 1 |

### Core Files (Required for Weather)

| File | Purpose |
|------|---------|
| [weather-core.yaml](weather-core.yaml) | Globals, scripts, switches |
| [weather-fonts-text.yaml](weather-fonts-text.yaml) | Icon fonts + text sensors |
| [psram-helpers.yaml](psram-helpers.yaml) | PSRAM buffers (REQUIRED) |

## Sensor Breakdown

### weather-sensors-current.yaml (8 sensors)
**Current Weather:**
1. Current Temperature
2. Feels Like Temperature
3. Precipitation Today

**Day 1 Forecast:**
4. High Temperature
5. Low Temperature
6. Condition (with icon)
7. Date
8. Precipitation Probability

**Memory:** ~5KB
**BLE Compatible:** ✅ YES!

---

### weather-sensors-daily.yaml (45 sensors)
**Days 2-10 (9 days × 5 sensors):**
- High Temperature (9)
- Low Temperature (9)
- Condition with icon (9)
- Date (9)
- Precipitation Probability (9)

**Memory:** ~30KB
**BLE Compatible:** ⚠️ Maybe (tight - Current + Daily + BLE = ~95KB total)

---

### weather-sensors-hourly.yaml (72 sensors)
**Hours 1-6 (6 hours × 12 sensors):**
- Date/Time (6)
- Temperature (6)
- Apparent Temperature (6)
- Condition with icon (6)
- Humidity (6)
- Precipitation (6)
- Precipitation Probability (6)
- Wind Speed (6)
- Wind Direction (6)
- Pressure (6)
- UV Index (6)
- Cloud Cover (6)

**Memory:** ~45KB
**BLE Compatible:** ❌ NO (too much)

## Configuration Examples

### Configuration 1: BLE + Minimal Weather (RECOMMENDED for BLE)
```yaml
packages:
  # Essential
  psram_helpers: !include packages/psram-helpers.yaml
  memory_stats: !include packages/memory-stats.yaml

  # BLE
  ble_coexist_tuning: !include packages/ble-coexist-tuning.yaml
  ble_core_simplified: !include packages/ble-core-simplified.yaml

  # Weather - MINIMAL (just current + Day 1)
  weather_core: !include packages/weather-core.yaml
  weather_fonts_text: !include packages/weather-fonts-text.yaml
  weather_sensors_current: !include packages/weather-sensors-current.yaml

  # Other features
  airq_core: !include packages/airq-core.yaml
  wifi_core: !include packages/wifi-core.yaml

  # Core
  remote_package: !include Halo-v1-Core.yaml

# LVGL Pages - need to create custom package with:
#   - vertical_clock_page
#   - AirQ_page
#   - WiFi_page (from wifi-core)
#   - weather_forecast_page (from weather-page-forecast.yaml)

lvgl:
  buffer_size: 15%
  pages:
    # Include your pages here or use a pages package
    # You'll need to manually create a pages package that includes:
    - !include packages/weather-page-forecast.yaml
```

**Memory Budget:**
- Base + LVGL 15%: ~55KB
- AirQ: ~20KB
- WiFi: ~5KB
- BLE: ~60KB
- Weather Current: ~5KB
- Weather Core + Fonts: ~25KB
- **Total: ~170KB**
- **Free: ~40KB** ⚠️ Tight but should work

---

### Configuration 2: Current + Daily Weather (NO BLE)
```yaml
packages:
  # Essential
  psram_helpers: !include packages/psram-helpers.yaml
  memory_stats: !include packages/memory-stats.yaml

  # NO BLE
  ble_stubs: !include packages/ble-stubs.yaml

  # Weather - Current + Daily
  weather_core: !include packages/weather-core.yaml
  weather_fonts_text: !include packages/weather-fonts-text.yaml
  weather_sensors_current: !include packages/weather-sensors-current.yaml
  weather_sensors_daily: !include packages/weather-sensors-daily.yaml

  # Other features
  airq_core: !include packages/airq-core.yaml
  wifi_core: !include packages/wifi-core.yaml

  # Core
  remote_package: !include Halo-v1-Core.yaml

lvgl:
  buffer_size: 25%
  pages:
    # Include weather pages
    - !include packages/weather-page-forecast.yaml
    - !include packages/weather-page-daily.yaml
    - !include packages/weather-page-days-4-5.yaml
    - !include packages/weather-page-days-6-7.yaml
    - !include packages/weather-page-days-8-9.yaml
    - !include packages/weather-page-day-10.yaml
```

**Memory Budget:**
- Base + LVGL 25%: ~78KB
- AirQ: ~20KB
- WiFi: ~5KB
- Weather Current: ~5KB
- Weather Daily: ~30KB
- Weather Core + Fonts: ~25KB
- **Total: ~163KB**
- **Free: ~47KB** ✅ Comfortable

---

### Configuration 3: Full Weather (NO BLE)
```yaml
packages:
  # Essential
  psram_helpers: !include packages/psram-helpers.yaml
  memory_stats: !include packages/memory-stats.yaml

  # NO BLE
  ble_stubs: !include packages/ble-stubs.yaml

  # Weather - ALL
  weather_core: !include packages/weather-core.yaml
  weather_fonts_text: !include packages/weather-fonts-text.yaml
  weather_sensors_current: !include packages/weather-sensors-current.yaml
  weather_sensors_daily: !include packages/weather-sensors-daily.yaml
  weather_sensors_hourly: !include packages/weather-sensors-hourly.yaml

  # Other features
  airq_core: !include packages/airq-core.yaml
  wifi_core: !include packages/wifi-core.yaml

  # Core
  remote_package: !include Halo-v1-Core.yaml

lvgl:
  buffer_size: 25%
  pages:
    # Include ALL weather pages
    - !include packages/weather-page-forecast.yaml
    - !include packages/weather-page-hourly.yaml
    - !include packages/weather-page-hourly-2.yaml
    - !include packages/weather-page-daily.yaml
    - !include packages/weather-page-days-4-5.yaml
    - !include packages/weather-page-days-6-7.yaml
    - !include packages/weather-page-days-8-9.yaml
    - !include packages/weather-page-day-10.yaml
```

**Memory Budget:**
- Base + LVGL 25%: ~78KB
- AirQ: ~20KB
- WiFi: ~5KB
- Weather Current: ~5KB
- Weather Daily: ~30KB
- Weather Hourly: ~45KB
- Weather Core + Fonts: ~25KB
- **Total: ~208KB**
- **Free: ~2KB** ⚠️ Very tight! May need to disable AirQ or WiFi

## Memory Compatibility Matrix

| Configuration | Current | Daily | Hourly | BLE | Total RAM | Free RAM | Status |
|--------------|---------|-------|--------|-----|-----------|----------|--------|
| BLE + Current | ✅ | ❌ | ❌ | ✅ | ~170KB | ~40KB | ⚠️ Tight |
| BLE + Current + Daily | ✅ | ✅ | ❌ | ✅ | ~200KB | ~10KB | ❌ Too tight |
| Current Only | ✅ | ❌ | ❌ | ❌ | ~110KB | ~100KB | ✅ Great |
| Current + Daily | ✅ | ✅ | ❌ | ❌ | ~140KB | ~70KB | ✅ Good |
| Current + Hourly | ✅ | ❌ | ✅ | ❌ | ~155KB | ~55KB | ✅ Good |
| Full Weather | ✅ | ✅ | ✅ | ❌ | ~185KB | ~25KB | ⚠️ Tight |
| Full Weather + AirQ | ✅ | ✅ | ✅ | ❌ | ~208KB | ~2KB | ❌ Too tight |

## Page Integration

The page files are designed to be included directly in your `lvgl:` section. However, ESPHome's package system has limitations with LVGL pages.

### Option 1: Direct Include (Simple)
```yaml
lvgl:
  pages:
    - !include packages/weather-page-forecast.yaml
    - !include packages/weather-page-daily.yaml
```

### Option 2: Custom Pages Package (Recommended)
Create a custom pages package (e.g., `lvgl-pages-custom.yaml`) that includes:
1. Clock page
2. AirQ page (if enabled)
3. WiFi page (if enabled)
4. Selected weather pages

Then include that package in your device file.

## Dependencies

### Required for ANY Weather
```yaml
psram_helpers: !include packages/psram-helpers.yaml
weather_core: !include packages/weather-core.yaml
weather_fonts_text: !include packages/weather-fonts-text.yaml
```

### Sensor Dependencies
- `weather-sensors-current.yaml` - Standalone (but needs weather-core)
- `weather-sensors-daily.yaml` - REQUIRES weather-sensors-current (uses Day 1 for reference)
- `weather-sensors-hourly.yaml` - REQUIRES weather-sensors-current

### Page Dependencies
- `weather-page-forecast.yaml` - REQUIRES weather-sensors-current
- `weather-page-daily.yaml` - REQUIRES weather-sensors-current + weather-sensors-daily
- `weather-page-hourly.yaml` - REQUIRES weather-sensors-current + weather-sensors-hourly

## Home Assistant Entities Required

### For weather-sensors-current.yaml:
```
sensor.st_00143056_temperature
sensor.st_00143056_feels_like
sensor.hhut_nearcast_precipitation_today
input_text.forecast_day_1_high_temperature
input_text.forecast_day_1_low_temperature
input_text.forecast_day_1_condition
input_text.forecast_day_1_date
input_text.forecast_day_1_precipitation_probability
```

### For weather-sensors-daily.yaml:
```
input_text.forecast_day_N_high_temperature (N = 2-10)
input_text.forecast_day_N_low_temperature (N = 2-10)
input_text.forecast_day_N_condition (N = 2-10)
input_text.forecast_day_N_date (N = 2-10)
input_text.forecast_day_N_precipitation_probability (N = 2-10)
```

### For weather-sensors-hourly.yaml:
```
input_text.forecast_hour_N_date (N = 1-6)
input_text.forecast_hour_N_temperature (N = 1-6)
input_text.forecast_hour_N_apparent_temperature (N = 1-6)
input_text.forecast_hour_N_condition (N = 1-6)
input_text.forecast_hour_N_humidity (N = 1-6)
input_text.forecast_hour_N_precipitation (N = 1-6)
input_text.forecast_hour_N_precipitation_probability (N = 1-6)
input_text.forecast_hour_N_wind_speed (N = 1-6)
input_text.forecast_hour_N_wind_direction (N = 1-6)
input_text.forecast_hour_N_pressure (N = 1-6)
input_text.forecast_hour_N_uv_index (N = 1-6)
input_text.forecast_hour_N_cloud_cover (N = 1-6)
```

## Migration from Old Weather System

### From Full Weather (OLD):
**Before:**
```yaml
weather_core: !include packages/weather-core.yaml
weather_sensors: !include packages/weather-sensors.yaml  # 125 sensors!
weather_fonts_text: !include packages/weather-fonts-text.yaml
```

**After (Current Only):**
```yaml
weather_core: !include packages/weather-core.yaml
weather_sensors_current: !include packages/weather-sensors-current.yaml  # 8 sensors
weather_fonts_text: !include packages/weather-fonts-text.yaml
```
**Saves: ~75KB RAM!**

**After (Current + Daily):**
```yaml
weather_core: !include packages/weather-core.yaml
weather_sensors_current: !include packages/weather-sensors-current.yaml  # 8 sensors
weather_sensors_daily: !include packages/weather-sensors-daily.yaml  # 45 sensors
weather_fonts_text: !include packages/weather-fonts-text.yaml
```
**Saves: ~30KB RAM!**

## Recommendations by Use Case

### "I want BLE + some weather"
→ Use **Configuration 1**: BLE + Current only
- 8 sensors, ~5KB RAM
- Just main forecast page
- Free RAM: ~40KB (tight but works)

### "I want full weather, don't care about BLE"
→ Use **Configuration 2**: Current + Daily (NO BLE)
- 53 sensors, ~35KB RAM
- All forecast pages
- Free RAM: ~47KB (comfortable)

### "I want everything!"
→ **Not possible** - Pick BLE OR Full Weather

### "I want hourly forecasts"
→ Use **Configuration 3**: Current + Hourly (NO BLE)
- 80 sensors, ~50KB RAM
- Current + Hourly pages
- Free RAM: ~55KB (good)
- Note: Hourly uses more sensors than daily!

## Troubleshooting

### "Out of memory" with BLE + Current
**Problem:** Even minimal weather too much with BLE
**Solutions:**
1. Disable AirQ: `airq_stubs: !include packages/airq-stubs.yaml`
2. Disable WiFi monitoring: `wifi_stubs: !include packages/wifi-stubs.yaml`
3. Reduce LVGL buffer: `buffer_size: 12%` (minimum)

### "Widget not found" errors
**Problem:** Page references widget that doesn't exist
**Solution:** Make sure you've included the correct sensor file
- Forecast page → needs weather-sensors-current
- Daily pages → needs weather-sensors-current + weather-sensors-daily
- Hourly pages → needs weather-sensors-current + weather-sensors-hourly

### "Entity not found" in Home Assistant
**Problem:** Sensor trying to read HA entity that doesn't exist
**Solution:** Update entity IDs in sensor files to match your HA setup

## Summary

You now have complete control over weather features:

**Sensor Modules:**
- ✅ weather-sensors-current.yaml (8 sensors, ~5KB) - BLE compatible!
- ✅ weather-sensors-daily.yaml (45 sensors, ~30KB)
- ✅ weather-sensors-hourly.yaml (72 sensors, ~45KB)

**Page Modules:**
- ✅ weather-page-forecast.yaml - Main forecast
- ✅ weather-page-daily.yaml + 4 more - Multi-day pages
- ✅ weather-page-hourly.yaml + 1 more - Hourly pages

**Mix and match to fit your memory budget!**

**For BLE:** Use only weather-sensors-current (8 sensors)
**For Full Weather:** Use all three sensor files (125 sensors total)
**Sweet Spot:** Current + Daily (53 sensors, ~35KB RAM)
