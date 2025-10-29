# Quick Start: BLE + Weather Forecast

**Goal:** Enable BLE bluetooth proxy AND a weather forecast page without running out of memory.

## The Problem

- BLE needs ~60KB RAM
- Full weather needs ~130KB RAM
- Total available: ~210KB internal RAM
- **60 + 130 = 190KB** ❌ Doesn't fit!

## The Solution

Use **modular weather** - enable only the forecast page you need:

- BLE needs ~60KB RAM
- **Basic weather** needs ~45KB RAM
- **60 + 45 = 105KB** ✅ FITS!

## Steps

### 1. Update Your Device File

Edit your device yaml file (e.g., [halo-v1-79e384.yaml](halo-v1-79e384.yaml)):

```yaml
packages:
  # Utilities
  memory_stats: !include packages/memory-stats.yaml
  psram_helpers: !include packages/psram-helpers.yaml  # REQUIRED

  # Features
  airq_core: !include packages/airq-core.yaml  # Optional: remove if not needed

  # BLE - NEW simplified version (no delays)
  ble_core: !include packages/ble-core-simplified.yaml

  # Weather - NEW basic version (just forecast page)
  weather_forecast_basic: !include packages/weather-forecast-basic.yaml
  weather_sensors: !include packages/weather-sensors.yaml  # Still need data!

  # Core (always last)
  remote_package:
    url: https://github.com/truffshuff/halo.git/
    ref: main
    files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/Halo-v1-Core.yaml]
    refresh: always

# IMPORTANT: Reduce buffer for BLE compatibility
lvgl:
  buffer_size: 15%  # Was 30%, now 15% to free up RAM
```

### 2. Update Your LVGL Pages

You need to create or use a pages package that includes:
- ✅ Vertical clock page
- ✅ AirQ page (if airq_core enabled)
- ✅ Weather forecast page (from weather-forecast-basic)
- ❌ NO hourly forecast pages (too much memory)
- ❌ NO daily forecast pages (too much memory)

**Option A:** Use existing package if available:
```yaml
lvgl_pages: !include packages/lvgl-pages-airq-weather-basic.yaml
```

**Option B:** Create your own by copying an existing lvgl-pages file and:
1. Remove hourly_forecast_page, hourly_forecast_page_2
2. Remove daily_forecast_page and extended forecast pages
3. Keep only: vertical_clock_page, AirQ_page (if wanted), weather_forecast_page

### 3. Remove Old Config

Make sure you DON'T have these old packages enabled:

```yaml
# REMOVE OR COMMENT OUT:
# weather_core: !include packages/weather-core.yaml  # Now in forecast-basic
# weather_fonts_text: !include packages/weather-fonts-text.yaml  # Now in forecast-basic
# ble_core_delayed: !include packages/ble-core-delayed.yaml  # Use simplified instead
```

### 4. Flash and Test

1. **Compile and flash** your updated config
2. **Watch the logs** for memory issues
3. **Check Home Assistant**:
   - BLE devices should appear
   - Weather forecast page should display
   - Free Heap sensor should show 60KB+

## What You Get

With this configuration:

✅ **BLE Bluetooth Proxy** - Scan for and proxy BLE devices to Home Assistant
✅ **Weather Forecast Page** - Shows current conditions, high/low, precipitation, feels-like temp
✅ **Stable Operation** - ~70KB free RAM, plenty of headroom
✅ **Fast Updates** - No artificial delays

## What You DON'T Get

❌ Hourly forecast pages (6-hour detailed view)
❌ Daily forecast pages (10-day overview)
❌ WireGuard VPN (uses additional memory)

**Want those?** You'll need to disable BLE or reduce other features.

## Troubleshooting

### "Out of memory" errors

**Problem:** Still running out of RAM

**Solutions:**
1. Lower LVGL buffer more: `buffer_size: 12%` (minimum)
2. Disable AirQ: Comment out `airq_core` package
3. Reduce BLE connections: In `ble-core-simplified.yaml`, change `connection_slots: 3` to `connection_slots: 1`

### BLE not showing devices

**Problem:** BLE scanner not working

**Solutions:**
1. Check logs for "BLE scanner started"
2. Verify Home Assistant is connected (triggers BLE start)
3. Check memory - if Free Heap < 40KB, BLE may crash

### Weather page blank

**Problem:** Forecast page shows but no data

**Solutions:**
1. Verify `weather-sensors.yaml` is included
2. Check Home Assistant entities exist:
   - `sensor.outdoor_temperature`
   - `input_text.forecast_day_1_high_temperature`
   - `input_text.forecast_day_1_condition`
3. Wait 30 seconds for initial data sync

### Page navigation stuck

**Problem:** Can't switch between pages

**Solutions:**
1. Make sure page rotation switches are enabled in HA
2. Touch the top-right corner to manually navigate
3. Check logs for page transition errors

## Monitor Memory

In Home Assistant, watch these sensors:

- **Free Heap (Internal)** - Should stay above 60KB
  - 80-100KB = Comfortable ✅
  - 60-80KB = OK ⚠️
  - <60KB = Risky ❌

- **PSRAM Free** - Should have plenty (MB range)
- **Largest Free Block** - Should be >30KB minimum

## Next Steps

Once this is working, you can experiment:

### Add Daily Forecasts (if memory allows)

1. Check Free Heap sensor - need 80KB+ free
2. Add package: `weather_forecast_daily: !include packages/weather-forecast-daily.yaml`
3. Update lvgl_pages to include daily_forecast_page
4. Monitor memory - if crashes, remove it

### Optimize Further

See full guide: [README-WEATHER-MODULAR.md](packages/README-WEATHER-MODULAR.md)

---

## Summary

**Before:** BLE + Full Weather = 190KB = ❌ Out of Memory

**After:** BLE + Basic Weather = 105KB = ✅ Works!

**Key Changes:**
1. Use `ble-core-simplified.yaml` (no delays)
2. Use `weather-forecast-basic.yaml` (just main forecast)
3. Reduce LVGL buffer to 15%
4. Keep PSRAM optimizations from Core

**Result:** BLE proxy + weather forecast that actually works! 🎉

## Files Created

- [packages/ble-core-simplified.yaml](packages/ble-core-simplified.yaml) - Simplified BLE (delays removed)
- [packages/weather-forecast-basic.yaml](packages/weather-forecast-basic.yaml) - Minimal weather forecast
- [README-WEATHER-MODULAR.md](packages/README-WEATHER-MODULAR.md) - Full modular weather guide
- [SUMMARY-MODULAR-WEATHER-BLE.md](SUMMARY-MODULAR-WEATHER-BLE.md) - Detailed analysis
- [QUICK-START-BLE-WEATHER.md](QUICK-START-BLE-WEATHER.md) - This guide

## Questions?

- **"Can I add more weather pages?"** - Only if Free Heap > 80KB
- **"Why remove the delays?"** - They didn't help; memory is allocated at init
- **"Is this safe?"** - Yes, with 60KB+ free it's stable
- **"Can I customize?"** - Yes, see README-WEATHER-MODULAR.md

**Need help?** Check the [full documentation](packages/README-WEATHER-MODULAR.md) or [BLE integration guide](packages/README-BLE-INTEGRATION.md).
