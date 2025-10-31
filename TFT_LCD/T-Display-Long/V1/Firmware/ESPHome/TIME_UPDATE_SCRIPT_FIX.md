# Time Update Script Duplicate Definition Fix

## Problem

When compiling the WiFi-only configuration, you received this error:

```
script: [source /config/esphome/packages/vertical-clock-core.yaml:97]
ID time_update redefined! Check script->2->id.
id: time_update
```

Additionally, there was a reference to `halo_time` which doesn't exist (should be `ha_time`).

## Root Cause

The `lvgl-pages-*.yaml` packages were incorrectly DEFINING the `time_update` script themselves, in addition to the dedicated `time-update-*.yaml` packages also defining it. This caused duplicate ID errors when both were included.

The architecture was wrong:
- **Correct:** `time-update-*.yaml` defines the script, `lvgl-pages-*.yaml` only uses it
- **Was happening:** Both files were defining `time_update`

## Solution

Removed all `time_update` script definitions from `lvgl-pages-*.yaml` files and replaced them with documentation comments explaining that the script is provided by the time-update package.

## Files Fixed

### Direct edits:
- `lvgl-pages-wifi-only.yaml` - Removed duplicate script definition

### Automated fixes (via Python script):
- `lvgl-pages-airq-weather-forecast-only.yaml`
- `lvgl-pages-airq-wifi-weather-forecast.yaml`
- `lvgl-pages-weather-forecast-daily.yaml`
- `lvgl-pages-weather-forecast-hourly.yaml`
- `lvgl-pages-weather-forecast-only.yaml`
- `lvgl-pages-wifi-weather-forecast-only.yaml`

### Not affected (no duplicate found):
- `lvgl-pages-clock-only.yaml`
- `lvgl-pages-airq-only.yaml`
- `lvgl-pages-airq-wifi.yaml`
- `lvgl-pages-airq-weather-forecast-only.yaml` (weather variant)
- `lvgl-pages-wifi-wireguard.yaml`
- `lvgl-pages-wireguard-only.yaml`
- `lvgl-pages-airq-wifi-wireguard.yaml`
- `lvgl-pages-full-wireguard.yaml`

## New Architecture

### Time Update Packages (`time-update-*.yaml`)
**Responsibility:** Define the `time_update` script
- `time-update-clock-only.yaml` - Updates vertical clock only
- `time-update-wifi-only.yaml` - Updates vertical clock, WiFi status
- `time-update-wireguard-only.yaml` - Updates vertical clock with SNTP/HA switching
- `time-update-basic.yaml` - Updates clock + AirQ page (optional)
- `time-update-wireguard.yaml` - Updates clock + AirQ + SNTP/HA switching

### LVGL Pages Packages (`lvgl-pages-*.yaml`)
**Responsibility:** Define pages and page rotation logic
- DO define LVGL pages
- DO define page rotation handler (interval)
- DO NOT define time_update script
- DO reference time_update script (via `script.execute: time_update`)

### Core Package (`Halo-v1-Core.yaml`)
**Responsibility:** Core device configuration
- Defines `page_transition_cleanup` script
- Does NOT define `time_update` script

## How It Works Now

1. User selects a time-update package based on their features
2. That package DEFINES the `time_update` script
3. LVGL display packages (vertical-clock-display.yaml, airq-display.yaml) CALL the `time_update` script
4. LVGL pages packages orchestrate which pages are shown
5. No conflicts, clean separation of concerns

## Testing

Your WiFi-only configuration should now compile successfully without:
- ❌ "ID time_update redefined" errors
- ❌ "Couldn't find ID 'halo_time'" errors (not in these files)

✅ The device should compile and run correctly.

## Future Prevention

To avoid this issue in the future:
1. **Time-update packages:** Should ONLY define the `time_update` script
2. **LVGL pages packages:** Should ONLY define pages and page rotation, NOT scripts
3. **Display packages:** Should only update existing widgets, not define global scripts
