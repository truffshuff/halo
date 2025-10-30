# ESPHome 2025.10.0 Upgrade Guide for Halo V1

## Overview

This guide covers the revolutionary new approach to weather data management using ESPHome 2025.10.0's Home Assistant Action Response feature. This enables **BLE + Full Weather coexistence** which was previously impossible due to memory constraints.

## What's New in ESPHome 2025.10.0

### 1. Home Assistant Action Responses
- **Bidirectional communication** with Home Assistant
- Call HA services and receive response data
- Process responses in ESPHome automations
- Use in lambda functions for display updates

### 2. Memory & Performance Optimizations
- **~1-2KB RAM savings** on ESP32 across the board
- Replaced heavy C++ STL containers with lighter alternatives
- Logger: 35-72% faster
- BLE Server: 26x faster lookups, 1KB flash savings
- Zero-copy techniques in API calls

### 3. Dynamic Auto-Loading
- Components conditionally load dependencies based on configuration
- Reduces unnecessary component inclusion
- Saves memory automatically

---

## The Revolutionary Change: Weather via HA Actions

### Old Approach (Sensor-Based)
```yaml
# 125 persistent sensors
sensor:
  - platform: homeassistant
    entity_id: input_text.forecast_day_1_high_temperature
    # ... 124 more sensors ...
```

**Memory Impact:**
- 125 sensors × ~600 bytes = **~75KB RAM** (always allocated)
- BLE (60KB) + Weather (75KB) = **135KB** ❌ Too tight!

### New Approach (HA Actions)
```yaml
# On-demand data fetching
script:
  - id: fetch_weather_data
    then:
      - homeassistant.action:
          action: weather.get_forecasts
          data:
            type: daily
          target:
            entity_id: weather.home
          response_variable: forecast_data
      - lambda: |-
          // Process response and update display
```

**Memory Impact:**
- ~3KB RAM (only when fetching)
- BLE (60KB) + Weather Actions (3KB) = **63KB** ✅ Plenty of room!
- **Memory savings: ~70KB+**

---

## Implementation Options

### Option 1: Full HA Actions Approach (Recommended for BLE)

**Use Case:** You want BLE + Weather coexistence

**Files:**
- [weather-ha-actions.yaml](packages/weather-ha-actions.yaml) - New on-demand weather
- [halo-v1-template-dynamic.yaml](halo-v1-template-dynamic.yaml) - Template with feature flags

**Pros:**
- ✅ BLE + Full Weather works!
- ✅ ~70KB memory savings
- ✅ Data only loaded when needed
- ✅ Cleaner architecture

**Cons:**
- ⚠️ Requires ESPHome 2025.10.0+
- ⚠️ Requires updating weather page YAMLs to use globals
- ⚠️ Different data access pattern

**Setup:**
1. Upgrade to ESPHome 2025.10.0+
2. Use `halo-v1-template-dynamic.yaml` as your base
3. Enable features via substitutions:
   ```yaml
   substitutions:
     feature_ble: "true"
     feature_weather: "true"
     weather_entity_id: "weather.home"
   ```
4. Uncomment the `weather_ha_actions` package
5. Update your weather page YAMLs (see migration guide below)

### Option 2: Hybrid Approach

**Use Case:** You want some sensors + some HA actions

**Setup:**
- Use `weather-sensors-current.yaml` (8 sensors, ~5KB) for current weather
- Use HA actions for daily/hourly forecasts
- Still achieves significant memory savings

### Option 3: Keep Sensor Approach with Optimizations

**Use Case:** You don't need BLE, or can't upgrade to 2025.10.0 yet

**Files:**
- Keep existing `weather-sensors-*.yaml` files
- Use optimized `psram-helpers.yaml` (delays removed)

**Benefits:**
- ✅ ESPHome 2025.10.0 still gives you ~1-2KB savings
- ✅ Removed delays speed up boot time
- ✅ No code changes needed

---

## Migration Guide: Sensors → HA Actions

### Step 1: Update Your Device YAML

**Remove old packages:**
```yaml
# REMOVE THESE:
# weather_sensors_current: !include packages/weather-sensors-current.yaml
# weather_sensors_daily: !include packages/weather-sensors-daily.yaml
# weather_sensors_hourly: !include packages/weather-sensors-hourly.yaml
```

**Add new package:**
```yaml
# ADD THIS:
weather_ha_actions:
  url: https://github.com/truffshuff/halo.git/
  ref: main
  files:
    - TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/weather-ha-actions.yaml
  refresh: always
```

### Step 2: Configure Weather Entity

In `weather-ha-actions.yaml`, set your Home Assistant weather entity:
```yaml
substitutions:
  weather_entity_id: "weather.home"  # Change to your entity
  weather_refresh_interval: "300"    # 5 minutes
```

### Step 3: Update Weather Page YAMLs

Your existing weather pages use sensor IDs like `id(forecast_day1_high_temp).state`. These need to change to access the new global arrays.

**OLD (Sensor-based):**
```yaml
- lvgl.label.update:
    id: lbl_forecast_day1_high
    text: !lambda |-
      char buf[16];
      snprintf(buf, sizeof(buf), "%.0f°F", id(forecast_day1_high_temp).state);
      return buf;
```

**NEW (HA Actions with globals):**
```yaml
- lvgl.label.update:
    id: lbl_forecast_day1_high
    text: !lambda |-
      char buf[16];
      snprintf(buf, sizeof(buf), "%.0f°F", id(weather_forecast_temp_high)[0]);
      return buf;
```

**Data Access Mapping:**

| Old Sensor ID | New Global Access |
|---------------|-------------------|
| `forecast_day1_high_temp.state` | `weather_forecast_temp_high[0]` |
| `forecast_day2_high_temp.state` | `weather_forecast_temp_high[1]` |
| `forecast_day1_low_temp.state` | `weather_forecast_temp_low[0]` |
| `forecast_day1_condition.state` | `weather_forecast_condition[0]` |
| `forecast_day1_precip_prob.state` | `weather_forecast_precip_prob[0]` |
| `hourly_temp_1.state` | `weather_hourly_temp[0]` |
| `hourly_condition_1.state` | `weather_hourly_condition[0]` |
| `weather_today_outdoor_temp.state` | `weather_current_temp` |
| `weather_current_condition.state` | `weather_current_condition` |

### Step 4: Add Page Tracking (Optional but Recommended)

Add to your weather page YAML to enable auto-refresh:

```yaml
lvgl:
  pages:
    - id: weather_forecast_page
      # ... your page configuration ...
      # ADD THIS:
      on_load:
        - lambda: |-
            id(weather_on_weather_page) = true;
            // Refresh data when page loads
            id(fetch_weather_data).execute();
```

### Step 5: Customize Display Updates

Edit the `update_weather_display` script in `weather-ha-actions.yaml` to match your label IDs:

```yaml
script:
  - id: update_weather_display
    mode: single
    then:
      - lambda: |-
          // YOUR LABEL IDS HERE
          if (!isnan(id(weather_current_temp))) {
            char temp_str[16];
            snprintf(temp_str, sizeof(temp_str), "%.0f°F", id(weather_current_temp));
            id(lbl_weather_today_outdoor_temp).set_text(temp_str);
          }
          // ... more label updates ...
```

---

## Feature Flag Configuration

The new `halo-v1-template-dynamic.yaml` uses substitutions for easy feature toggling:

```yaml
substitutions:
  # Core Features
  feature_clock: "true"              # Always enabled
  feature_airq: "false"              # Air quality page
  feature_wifi_page: "false"         # WiFi info page
  feature_wireguard: "false"         # WireGuard VPN

  # Weather (NEW!)
  feature_weather: "true"            # Enable HA actions weather
  weather_entity_id: "weather.home"  # Your HA weather entity
  weather_refresh_interval: "300"    # Auto-refresh interval (seconds)

  # Connectivity
  feature_ble: "true"                # BLE presence detection

  # Optimization
  feature_psram_optimization: "true" # Keep enabled
  feature_memory_stats: "true"       # Memory monitoring
```

**To enable a feature:**
1. Change its flag to `"true"`
2. Uncomment the corresponding package section
3. Select the appropriate `lvgl_pages` package

---

## Memory Budget with New Approach

### ESP32-S3 Internal RAM: ~210KB

| Configuration | Memory Usage | Free RAM | BLE Compatible |
|--------------|--------------|----------|----------------|
| **Clock Only** | ~45KB | ~165KB | ✅ Yes |
| **Clock + BLE** | ~105KB | ~105KB | ✅ Yes |
| **Clock + Weather (HA Actions)** | ~48KB | ~162KB | ✅ Yes |
| **Clock + BLE + Weather (HA Actions)** | ~108KB | ~102KB | ✅ YES! 🎉 |
| **Clock + BLE + AirQ** | ~125KB | ~85KB | ✅ Yes |
| **Clock + BLE + Weather + AirQ** | ~128KB | ~82KB | ✅ Yes! |
| **OLD: Clock + Weather (Sensors)** | ~120KB | ~90KB | ❌ No |
| **OLD: Clock + BLE + Weather (Sensors)** | ~180KB | ~30KB | ❌ NO! |

**Key Insight:** The HA Actions approach enables configurations that were previously impossible!

---

## Testing Checklist

### Before Upgrading:
- [ ] Backup your current working configuration
- [ ] Note your current free heap: `heap_caps_get_free_size(MALLOC_CAP_INTERNAL)`
- [ ] Document any custom sensor entity IDs

### After Upgrading:
- [ ] Verify weather data fetches: Check logs for "Fetching weather data from Home Assistant..."
- [ ] Confirm display updates: Weather should show on pages
- [ ] Check memory usage: Should see ~70KB more free RAM
- [ ] Test BLE functionality: BLE devices should be detected
- [ ] Verify auto-refresh: Weather updates every N minutes on weather page
- [ ] Test manual refresh: Press "Refresh Weather" button

### Log Messages to Look For:
```
[weather] Fetching weather data from Home Assistant...
[weather] Processing daily forecast response...
[weather] Found 10 days of forecast data
[weather] ✓ Daily forecast data stored
[weather] Processing hourly forecast response...
[weather] Found 24 hours of forecast data
[weather] ✓ Hourly forecast data stored
[weather] Processing current weather...
[weather] ✓ Weather data fetch complete!
[weather] Current: 72.0°F, sunny
```

---

## Troubleshooting

### Weather Data Not Fetching

**Problem:** No weather data appears, logs show errors

**Solutions:**
1. Verify your `weather_entity_id` is correct:
   ```bash
   # In Home Assistant Developer Tools > States
   # Look for: weather.home, weather.openweathermap, etc.
   ```

2. Check ESPHome version:
   ```bash
   esphome version
   # Should be 2025.10.0 or later
   ```

3. Verify API connection:
   ```yaml
   api:
     encryption:
       key: !secret api_encryption_key
   ```

### Display Not Updating

**Problem:** Weather fetches but display doesn't update

**Solutions:**
1. Check label IDs in `update_weather_display` script
2. Verify globals are being populated (check logs)
3. Add debug logging:
   ```cpp
   ESP_LOGI("weather", "Temp high[0]: %.1f", id(weather_forecast_temp_high)[0]);
   ```

### Memory Still Tight

**Problem:** Free RAM not as high as expected

**Solutions:**
1. Verify you removed old sensor packages
2. Check LVGL buffer size (should be 15% with BLE):
   ```cpp
   -DLVGL_BUFFER_SIZE_PERCENTAGE=15
   ```
3. Enable memory stats package to diagnose
4. Ensure PSRAM optimization is enabled

### BLE + Weather Still Conflicts

**Problem:** BLE fails even with HA Actions approach

**Solutions:**
1. Verify you're using `weather-ha-actions.yaml` not sensor files
2. Check that old weather sensor packages are fully removed
3. Reduce LVGL buffer to 15%:
   ```yaml
   build_flags:
     - "-DLVGL_BUFFER_SIZE_PERCENTAGE=15"
   ```
4. Disable other heavy features temporarily to isolate

---

## Performance Comparison

### Boot Time
- **Old (with 60s delay):** ~70 seconds until weather available
- **New (HA actions):** ~5 seconds initial fetch, instant on page view

### Memory Efficiency
- **Old:** 75KB allocated at all times (even when not viewing weather)
- **New:** 3KB only when fetching, 0KB when idle

### Display Responsiveness
- **Old:** Slow updates due to 125 individual sensor callbacks
- **New:** Batch processing, single update cycle

### Network Efficiency
- **Old:** 125 separate HA API calls on boot
- **New:** 3 efficient service calls (daily, hourly, current)

---

## Advanced Customization

### Custom Refresh Logic

Add conditional refresh based on time of day:
```yaml
interval:
  - interval: 60s
    then:
      - lambda: |-
          auto time = id(homeassistant_time).now();
          // Only refresh during daytime hours
          if (time.hour >= 6 && time.hour <= 22 && id(weather_on_weather_page)) {
            id(fetch_weather_data).execute();
          }
```

### Selective Data Fetching

Fetch only what you need:
```yaml
# Only fetch daily forecast, skip hourly
script:
  - id: fetch_weather_light
    then:
      - homeassistant.action:
          action: weather.get_forecasts
          data:
            type: daily  # Skip hourly
          target:
            entity_id: ${weather_entity_id}
          response_variable: daily_forecast
```

### Weather Icons from Conditions

Map weather conditions to icon fonts:
```cpp
const char* weather_icon(const std::string& condition) {
  if (condition == "sunny") return "\U0000F185";
  if (condition == "cloudy") return "\U0000F0C4";
  if (condition == "rainy") return "\U0000F176";
  // ... more mappings
  return "\U0000F0C2";  // default
}
```

---

## Future Enhancements

### Planned Features:
1. **Weather alerts** via HA action responses
2. **Forecast graphs** using LVGL charts with on-demand data
3. **Multiple weather sources** with fallback
4. **Smart caching** to reduce HA API calls

### Community Contributions:
- Pre-built weather page YAMLs using HA actions
- Icon font mappings for popular weather integrations
- Battery-optimized refresh schedules

---

## Support & Resources

### Documentation:
- [ESPHome 2025.10.0 Changelog](https://esphome.io/changelog/2025.10.0.html)
- [Home Assistant Actions Documentation](https://esphome.io/components/api.html#homeassistant-action)
- [Halo Project Repository](https://github.com/truffshuff/halo)

### Getting Help:
- Open an issue on GitHub with logs and configuration
- Include your ESPHome version: `esphome version`
- Provide memory stats: `heap_caps_get_free_size(...)`

---

## Summary

The ESPHome 2025.10.0 upgrade brings revolutionary improvements to the Halo V1 project:

✅ **BLE + Weather coexistence** now fully possible
✅ **~70KB memory savings** with HA Actions approach
✅ **Faster boot times** with removed delays
✅ **Cleaner architecture** with on-demand data fetching
✅ **Feature flags** for easier configuration

**Recommended upgrade path:**
1. Start with `halo-v1-template-dynamic.yaml`
2. Enable features via substitutions
3. Migrate weather pages to use globals
4. Enjoy BLE + Full Weather! 🎉
