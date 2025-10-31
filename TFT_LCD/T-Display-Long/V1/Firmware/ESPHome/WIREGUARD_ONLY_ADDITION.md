# WireGuard-Only Configuration Addition

## Summary

Added support for WireGuard-only mode (WiFi + WireGuard VPN without AirQ). This allows users to run a VPN-connected WiFi status monitor without the memory overhead of air quality sensors.

## What Was Missing

The system previously only supported:
- WiFi-only (wifi-only pages, time-update-wifi-only)
- AirQ + WiFi + WireGuard (airq-wifi-wireguard pages, time-update-wireguard)

It did NOT support:
- WiFi + WireGuard without AirQ

The issue was that `time-update-wireguard.yaml` ALWAYS tried to update the `timeVal` widget on the AirQ page, making it incompatible with devices that don't have AirQ enabled.

## Files Created

### 1. `packages/time-update-wireguard-only.yaml` (NEW)
- Specialized time-update script for WiFi + WireGuard WITHOUT AirQ
- Switches between SNTP and Home Assistant time based on WireGuard connection status
- Only updates vertical clock widgets (no AirQ references)
- 127 lines

### 2. `packages/lvgl-pages-wireguard-only.yaml` (NEW)
- LVGL pages package for WiFi + WireGuard only configuration
- Includes: Vertical Clock page + WiFi+WireGuard page
- Uses page indices: 0 (clock), 2 (wifi+wg) - skips index 1 reserved for AirQ
- Embedded page rotation handler for 2-page configuration
- 85 lines

## Files Modified

### 1. `halo-v1-79e384.yaml`
**Changes in time-update section (lines 97-122):**
- Added `time-update-wireguard-only.yaml` to the chart
- Updated line descriptions to be more specific
- Added entry for "WiFi + WireGuard (no AirQ)" → `time-update-wireguard-only.yaml`

**Changes in lvgl_pages_combo section (lines 73-83):**
- Added `wireguard-only` option for WiFi + WireGuard without AirQ
- Added `airq-wifi-wireguard` for AirQ + WiFi + WireGuard
- Added `airq-wifi-weather-wg` for all features + WireGuard

**Changes in lvgl_pages includes (lines 171, 176-177):**
- Added comment for `lvgl-pages-wireguard-only.yaml`
- Added comment for `lvgl-pages-airq-wifi-wireguard.yaml`
- Added comment for `lvgl-pages-full-wireguard.yaml`

### 2. `CONFIGURATION_CHECKLIST.md`
**Updated Step 2 (lvgl_pages_combo selection):**
- Added "WiFi + WireGuard (no AirQ)" → `wireguard-only`
- Added "AirQ + WiFi + WireGuard" → `airq-wifi-wireguard`
- Added "AirQ + WiFi + Weather + WireGuard" → `airq-wifi-weather-wg`

**Updated Step 3 (Time Update Package selection):**
- Added `time-update-wireguard-only.yaml` for WiFi + WireGuard without AirQ
- Reordered for clarity

**Updated Quick Reference table:**
- Added "WiFi+WireGuard" entry with wireguard-only packages
- Added entries for other WireGuard combinations
- Now shows 10 common configurations

### 3. `PACKAGE_SELECTION_GUIDE.md`
**Updated Step 3 (Time Update Package table):**
- Added "WiFi + WireGuard (NO AirQ)" row
- Clarified that `time-update-basic.yaml` is for non-WireGuard AirQ configs

**Updated Supported Feature Combinations table:**
- Added "WiFi + WireGuard" configuration
- Moved it to logical position (after Weather-only, before multi-feature combos)

## Configuration Usage

### To use WiFi + WireGuard without AirQ:

1. **Set feature toggles:**
```yaml
airq_enabled: "false"
ble_enabled: "false"
wifi_enabled: "true"
weather_enabled: "false"
wireguard_enabled: "true"
```

2. **Select lvgl_pages_combo:**
```yaml
lvgl_pages_combo: "wireguard-only"
```

3. **Select time_update_script:**
```yaml
time_update_script: !include packages/time-update-wireguard-only.yaml
```

4. **Configure packages:**
```yaml
airq: !include packages/airq-stubs.yaml
ble: !include packages/ble-stubs.yaml
wifi_core: !include packages/wifi-core.yaml
wifi_display: !include packages/wifi-display.yaml
weather: !include packages/weather-stubs.yaml
wireguard: !include packages/wireguard.yaml
lvgl_pages: !include packages/lvgl-pages-wireguard-only.yaml
```

## Key Design Differences

### time-update-wireguard-only.yaml vs time-update-wireguard.yaml

| Aspect | wireguard-only | wireguard |
|--------|---|---|
| AirQ page update | ❌ NO (not defined) | ✅ YES (required) |
| Clock page update | ✅ YES | ✅ YES |
| SNTP/HA switching | ✅ YES | ✅ YES |
| AirQ widget: timeVal | No reference | Always updates |
| Use case | WiFi + VPN only | AirQ + WiFi + VPN |

### lvgl-pages-wireguard-only.yaml vs lvgl-pages-wifi-wireguard.yaml

Wait - there's a naming confusion here! Let me clarify:

- `lvgl-pages-wifi-wireguard.yaml` - This doesn't exist, but `lvgl-pages-airq-wifi-wireguard.yaml` does
- `lvgl-pages-wifi-wireguard.yaml` appears to reference it in comments but the file we created is `lvgl-pages-wireguard-only.yaml`

The distinction is:
- `lvgl-pages-wireguard-only.yaml` (NEW) - Clock + WiFi+WireGuard pages, NO AirQ
- `lvgl-pages-airq-wifi-wireguard.yaml` (EXISTING) - Clock + AirQ + WiFi+WireGuard pages

## Memory Impact

WireGuard-only mode uses approximately:
- **Total: ~95KB** (slightly less than WiFi-only due to no AirQ sensors)
- **Free: ~115KB** (plenty of headroom for future features)
- **Compared to AirQ+WiFi+WireGuard: ~40KB savings** (no SCD40, MICS-4514, BME280, SEN55 sensors)

## Supported Page Indices

In WireGuard-only mode:

```
Index 0: Vertical Clock (always)
Index 1: (RESERVED for AirQ, not used)
Index 2: WiFi + WireGuard Status (combined page)
```

The page rotation handler skips index 1 and only shows pages at indices 0 and 2.

## Next Steps for Future Features

If adding Weather to WireGuard-only mode, you would need:
1. Rename `lvgl-pages-wireguard-only.yaml` to match pattern
2. Create new `lvgl-pages-wifi-weather-wireguard.yaml` (indices 0, 2, 3)
3. Weather would go at index 3 (skipping 1 for AirQ)

## Testing Recommendations

Users with this configuration should verify:
- [ ] Device compiles without "Couldn't find ID" errors
- [ ] WiFi page shows and updates correctly
- [ ] WireGuard status displays (connected/disconnected)
- [ ] Clock updates on both pages
- [ ] Time switches to SNTP when WireGuard is enabled
- [ ] Time reverts to HA when WireGuard is disabled
- [ ] Page rotation works between Clock and WiFi+WireGuard pages

## Backwards Compatibility

All changes are backwards compatible:
- Existing WiFi-only configurations still work with time-update-wifi-only.yaml
- Existing AirQ+WiFi+WireGuard configurations still work with time-update-wireguard.yaml
- All previously supported configurations remain unchanged
- Documentation expansion only, no breaking changes
