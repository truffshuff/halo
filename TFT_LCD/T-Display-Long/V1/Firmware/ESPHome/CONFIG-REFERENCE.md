# Halo Configuration Reference Guide

Quick reference for configuring your Halo device with the complete template.

## Files

- **[halo-v1-template-complete.yaml](halo-v1-template-complete.yaml)** - Complete template with ALL options documented
- **[halo-v1-79e384.yaml](halo-v1-79e384.yaml)** - Example working configuration
- **[QUICK-START-BLE-WEATHER.md](QUICK-START-BLE-WEATHER.md)** - Quick setup for BLE + Weather

## Quick Configuration Selector

### Configuration A: BLE + Basic Weather (Recommended)
**Best for:** Using BLE bluetooth proxy with weather forecast

```yaml
packages:
  memory_stats: ...
  psram_helpers: ...
  airq_core: ...

  # BLE - NEW simplified
  ble_coexist_tuning: ...
  ble_core_simplified: ...

  # Weather - Basic only
  weather_forecast_basic: ...
  weather_sensors: ...

  wifi_core: ...
  remote_package: ...
  lvgl_pages: !include lvgl-pages-airq-weather-basic.yaml

lvgl:
  buffer_size: 15%
```

**Memory:** ~105KB used, ~105KB free ✅
**Features:** BLE proxy, current weather, AirQ, WiFi
**Free RAM:** ~105KB (safe)

---

### Configuration B: Full Weather (No BLE)
**Best for:** Maximum weather features without BLE

```yaml
packages:
  memory_stats: ...
  psram_helpers: ...
  airq_core: ...

  # NO BLE
  ble_stubs: ...

  # Weather - Full
  weather_core: ...
  weather_sensors: ...
  weather_fonts_text: ...

  wifi_core: ...
  remote_package: ...
  lvgl_pages: !include lvgl-pages-full.yaml

lvgl:
  buffer_size: 25%
```

**Memory:** ~140KB used, ~70KB free ✅
**Features:** All weather pages, AirQ, WiFi
**Free RAM:** ~70KB (safe)

---

### Configuration C: BLE Only (No Weather)
**Best for:** Using BLE with maximum memory available

```yaml
packages:
  memory_stats: ...
  psram_helpers: ...
  airq_core: ...

  # BLE
  ble_coexist_tuning: ...
  ble_core_simplified: ...

  # NO Weather
  weather_stubs: ...

  wifi_core: ...
  remote_package: ...
  lvgl_pages: !include lvgl-pages-airq-wifi.yaml

lvgl:
  buffer_size: 20%
```

**Memory:** ~90KB used, ~120KB free ✅
**Features:** BLE proxy, AirQ, WiFi
**Free RAM:** ~120KB (plenty!)

---

### Configuration D: Minimal (Clock + AirQ)
**Best for:** Simple display with air quality monitoring

```yaml
packages:
  memory_stats: ...
  psram_helpers: ...
  airq_core: ...

  # NO BLE
  ble_stubs: ...

  # NO Weather
  weather_stubs: ...

  # NO WiFi monitoring
  wifi_stubs: ...

  remote_package: ...
  lvgl_pages: !include lvgl-pages-airq-only.yaml

lvgl:
  buffer_size: 30%
```

**Memory:** ~60KB used, ~150KB free ✅
**Features:** Clock, AirQ only
**Free RAM:** ~150KB (maximum)

---

## Package Quick Reference

### Essential (Always Include)
```yaml
memory_stats: !include packages/memory-stats.yaml
psram_helpers: !include packages/psram-helpers.yaml
remote_package: !include Halo-v1-Core.yaml
```

### BLE Options (Choose ONE)
```yaml
# OPTION A: Disabled
ble_stubs: !include packages/ble-stubs.yaml

# OPTION B: Enabled (NEW simplified)
ble_coexist_tuning: !include packages/ble-coexist-tuning.yaml
ble_core_simplified: !include packages/ble-core-simplified.yaml

# OPTION C: Enabled (old/fallback)
ble_coexist_tuning: !include packages/ble-coexist-tuning.yaml
ble_core: !include packages/ble-core.yaml
```

### Weather Options (Choose ONE)
```yaml
# OPTION A: Disabled
weather_stubs: !include packages/weather-stubs.yaml

# OPTION B: Basic (NEW - for BLE compatibility)
weather_forecast_basic: !include packages/weather-forecast-basic.yaml
weather_sensors: !include packages/weather-sensors.yaml

# OPTION C: Full (old - no BLE)
weather_core: !include packages/weather-core.yaml
weather_sensors: !include packages/weather-sensors.yaml
weather_fonts_text: !include packages/weather-fonts-text.yaml
```

### AirQ Options (Choose ONE)
```yaml
# Enabled
airq_core: !include packages/airq-core.yaml

# Disabled
airq_stubs: !include packages/airq-stubs.yaml
```

### WiFi Monitoring Options (Choose ONE)
```yaml
# Enabled
wifi_core: !include packages/wifi-core.yaml

# Disabled
wifi_stubs: !include packages/wifi-stubs.yaml
```

## LVGL Buffer Sizes

| Configuration | Buffer | Reason |
|--------------|--------|--------|
| BLE + Basic Weather | 15% | BLE needs internal RAM |
| BLE Only | 20% | More room without weather |
| Full Weather | 25% | No BLE, more for display |
| Minimal | 30% | Maximum for smooth rendering |
| Clock Only | 35% | Absolute maximum |

## Memory Budget Calculator

| Feature | Memory Used |
|---------|-------------|
| Base System | ~20KB |
| LVGL 15% | ~35KB |
| LVGL 20% | ~47KB |
| LVGL 25% | ~58KB |
| LVGL 30% | ~70KB |
| AirQ | ~20KB |
| WiFi Monitoring | ~5KB |
| BLE | ~60KB |
| Weather Basic | ~45KB |
| Weather Full | ~130KB |
| WireGuard | ~15KB |

**Total Available:** ~210KB internal RAM
**Minimum Free:** 60KB (required for stability)
**Comfortable Free:** 80KB+

## Feature Compatibility Matrix

| Features | BLE | Weather Basic | Weather Full | Free RAM | Status |
|----------|-----|---------------|--------------|----------|--------|
| BLE + Basic Weather + AirQ | ✅ | ✅ | ❌ | ~85KB | ✅ Works |
| BLE + AirQ | ✅ | ❌ | ❌ | ~120KB | ✅ Works |
| Full Weather + AirQ | ❌ | ❌ | ✅ | ~70KB | ✅ Works |
| BLE + Full Weather | ✅ | ❌ | ✅ | ~20KB | ❌ OOM! |
| All Features | ✅ | ❌ | ✅ | <0KB | ❌ Won't compile |

## Common Configurations Summary

### For BLE Users
- **With weather:** Use Config A (BLE + Basic Weather) - buffer 15%
- **Without weather:** Use Config C (BLE Only) - buffer 20%
- **Absolute minimum:** Disable AirQ too - buffer 20%

### For Weather Users
- **With BLE:** Use Config A (Basic Weather only) - buffer 15%
- **Without BLE:** Use Config B (Full Weather) - buffer 25%
- **Maximum weather:** Disable AirQ too - buffer 27%

### For Simple Users
- **Just clock + sensors:** Use Config D (Minimal) - buffer 30%
- **Absolute minimum:** Clock only - buffer 35%

## Page Package Reference

| Package | Clock | AirQ | WiFi | Weather | Daily | Hourly |
|---------|-------|------|------|---------|-------|--------|
| lvgl-pages-clock-only.yaml | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ |
| lvgl-pages-airq-only.yaml | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ |
| lvgl-pages-airq-wifi.yaml | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ |
| lvgl-pages-weather.yaml | ✅ | ❌ | ❌ | ✅ | ✅ | ✅ |
| lvgl-pages-airq-weather.yaml | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ |
| lvgl-pages-full.yaml | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| lvgl-pages-full-wireguard.yaml | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| lvgl-pages-airq-weather-basic.yaml¹ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ |

¹ *Needs to be created - use for BLE + Basic Weather*

## Step-by-Step Setup

### 1. Copy Template
```bash
cp halo-v1-template-complete.yaml halo-v1-YOURNAME.yaml
```

### 2. Edit Basic Info
```yaml
esphome:
  name: halo-v1-YOURNAME
  friendly_name: "My Halo Display"
```

### 3. Choose Configuration
Pick from A, B, C, or D above and uncomment those packages.

### 4. Set Buffer Size
```yaml
lvgl:
  buffer_size: 15%  # Based on your configuration
```

### 5. Select Pages Package
```yaml
lvgl_pages: !include lvgl-pages-XXXX.yaml
```

### 6. Update Page Options
```yaml
select:
  - platform: template
    name: Default Page
    options:
      - "Vertical Clock"
      - "AirQ"  # Only if airq_core enabled
      - "Weather"  # Only if weather enabled
      # etc.
```

### 7. Flash and Monitor
```bash
esphome run halo-v1-YOURNAME.yaml
```

Watch the logs and check Free Heap sensor in Home Assistant.

## Troubleshooting Quick Reference

| Issue | Solution |
|-------|----------|
| Out of memory | Reduce buffer to 12%, disable a feature |
| BLE not scanning | Check Free Heap > 60KB, verify HA connected |
| Weather blank | Verify weather_sensors included, check HA entities |
| Pages missing | Check lvgl_pages matches enabled features |
| Compile error | Check package URLs, verify all dependencies |
| Won't boot | Flash via USB, check logs for specific error |

## Monitoring

### In Home Assistant
Watch these sensors:
- **Free Heap (Internal)** - Should stay > 60KB
- **Free PSRAM** - Should have plenty (MB range)
- **Largest Free Block** - Should be > 30KB

### In Logs
```
[memory_boot] Free internal: XXX bytes  # Should be > 60000
[ble] BLE scanner started               # If BLE enabled
[weather] Weather data updated          # If weather enabled
```

## Migration Paths

### From Old Full Weather to New Basic Weather
1. Comment out: `weather_core`, `weather_fonts_text`
2. Add: `weather_forecast_basic`
3. Keep: `weather_sensors` (still needed)
4. Update pages package (remove daily/hourly pages)
5. Memory savings: ~85KB

### From BLE-Delayed to BLE-Simplified
1. Replace: `ble-core-delayed.yaml` → `ble-core-simplified.yaml`
2. Remove any `ble_delay_seconds` substitutions
3. No other changes needed
4. Boot time: Faster (no 10s delay)

### Adding BLE to Existing Weather Setup
1. First: Switch to `weather-forecast-basic` (see above)
2. Then: Add `ble_coexist_tuning` and `ble_core_simplified`
3. Reduce: `buffer_size` to 15%
4. Test: Watch Free Heap sensor

## Links to Documentation

- **Quick Start:** [QUICK-START-BLE-WEATHER.md](QUICK-START-BLE-WEATHER.md)
- **Modular Weather:** [packages/README-WEATHER-MODULAR.md](packages/README-WEATHER-MODULAR.md)
- **Complete Analysis:** [SUMMARY-MODULAR-WEATHER-BLE.md](SUMMARY-MODULAR-WEATHER-BLE.md)
- **BLE Integration:** [packages/README-BLE-INTEGRATION.md](packages/README-BLE-INTEGRATION.md)
- **PSRAM Optimization:** [packages/README-PSRAM-OPTIMIZATION.md](packages/README-PSRAM-OPTIMIZATION.md)

## New Files Reference

| File | Purpose | Size |
|------|---------|------|
| halo-v1-template-complete.yaml | Complete template with all options | Config |
| packages/weather-forecast-basic.yaml | Minimal weather (just forecast page) | ~20KB |
| packages/ble-core-simplified.yaml | Simplified BLE (delays removed) | ~3KB |
| packages/weather-core-standalone.yaml | Weather core functionality | ~5KB |
| README-WEATHER-MODULAR.md | Modular weather guide | Doc |
| SUMMARY-MODULAR-WEATHER-BLE.md | Complete analysis | Doc |
| QUICK-START-BLE-WEATHER.md | Quick setup guide | Doc |
| CONFIG-REFERENCE.md | This file | Doc |

---

**Quick Decision Tree:**

1. Want BLE?
   - Yes → Go to 2
   - No → Use Config B (Full Weather)

2. Want Weather?
   - Yes → Use Config A (BLE + Basic Weather)
   - No → Use Config C (BLE Only)

3. Want maximum features?
   - Can't have BLE + Full Weather
   - Choose which is more important

**Result:** BLE + Weather is now possible with the modular system! 🎉
