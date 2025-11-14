# Phase 1 Progress - System Module Reorganization

**Date:** 2025-11-13 - 2025-11-14
**Branch:** modular
**Status:** 🚧 IN PROGRESS - Structure created, resolving compilation errors

---

## Summary

Phase 1 of the modularization plan is in progress. The existing foundation files (hardware.yaml, esphome_base.yaml, globals.yaml) have been reorganized into capability-based system modules. The new structure is in place, but we're still resolving duplicate component errors to achieve a clean compilation.

---

## What Was Created

### System Modules (5 files)

All system modules are located in `packages/system/`:

1. **esphome_core.yaml** (~124 lines)
   - External components (MUST be early - other modules need them)
   - ESP32 board configuration (ESP32-S3, 16MB flash, 240MHz)
   - PSRAM configuration (octal mode, 80MHz)
   - Memory management (SPIRAM, 64KB stack sizes for LVGL)
   - NimBLE/BLE sdkconfig options (ALL Bluedroid disabled, NimBLE enabled)
   - TCP/IP configuration (increased buffers for BLE)
   - Watchdog timeout (30s for HTTP/JSON operations)
   - **Intentional duplicates (needed early):**
     - external_components (also in hardware.yaml - legacy)

2. **display_hardware.yaml** (~126 lines)
   - SPI bus (quad SPI for display)
   - I2C buses (2 buses: lily_i2c, touch_i2c)
   - Display output (backlight PWM output only)
   - Display driver (AXS15231 AMOLED, 180x640)
   - Touchscreen driver (AXS15231, touch validation, auto-rotation reset)
   - **Removed to avoid duplicates:**
     - Light component (kept in weather-led-effects.yaml)
     - Display watchdog globals (kept in globals.yaml)
     - Display backlight binary sensor (kept in Core)
     - weather_helpers (kept in hardware.yaml)
   - **Notes added for Phase 2:**
     - Display watchdog will move here when globals.yaml deprecated
     - Backlight sensor will move here when Core broken down

3. **fonts_colors.yaml** (~280 lines) - NO CHANGES
   - 9 Montserrat font sizes (10-80pt)
   - 7 Material Design Icon font sizes (20-100pt)
   - 9 color definitions (my_red, my_green, etc.)
   - Image resources (ym_logo)

4. **system_management.yaml** (~54 lines)
   - HTTP request component (MUST be early - Core scripts need it)
   - Time sources (Home Assistant + SNTP - WireGuard needs them)
   - **Intentional duplicates (needed early):**
     - http_request (also in esphome_base.yaml - legacy)
     - time (also in esphome_base.yaml - legacy)
   - **Removed to avoid duplicates:**
     - Web server (kept in esphome_base.yaml)
   - **Notes added for Phase 2:**
     - API, system buttons, sensors will be added when extracted from Core

5. **networking.yaml** (~48 lines)
   - WiFi configuration (ssid, password from substitutions)
   - WiFi on_disconnect handler (resets startup blink counter)
   - WiFi on_connect handler (stops startup blink)
   - **Removed to avoid duplicates:**
     - Captive portal (kept in hardware.yaml)
   - **Notes added for Phase 2:**
     - WiFi sensors will be added when extracted from Core

### Feature Modules (1 file)

Feature module located in `packages/features/ble/`:

6. **ble_improv.yaml** (~84 lines)
   - nimble_base (MUST be early - nimble_improv needs it)
   - NimBLE Improv WiFi provisioning (authorized_duration, wifi_timeout)
   - Bluetooth proxy for Home Assistant
   - nimble_proxy component
   - cycleCounter global (for startup blink animation)
   - **Intentional duplicates (needed early):**
     - nimble_base (also in hardware.yaml - legacy)
   - **Removed to avoid duplicates:**
     - startup_light_blink switch (kept in Core)
   - **Notes added for Phase 2:**
     - RGB LED and blink intervals still in weather-led-effects.yaml
     - Will reorganize into BLE module or separate LED module

---

## What Was Changed

### Updated Files

1. **Halo-v1.yaml**
   - Updated package imports to use new system modules
   - Added clear comments explaining Phase 1 status
   - Marked legacy modules (globals.yaml, Core) for Phase 2 extraction
   - Import order:
     1. System modules (5 files)
     2. BLE feature module
     3. Legacy modules (globals, pages, weather, Core)

### Key Design Decisions

1. **Minimal Placeholder Modules**
   - Phase 1 modules are "placeholders" with minimal content
   - Only include components that MUST load early (before legacy files)
   - Accept intentional duplicates where load order requires it
   - Most functionality remains in legacy files until Phase 2

2. **Intentional Duplicates for Load Order**
   - `external_components` in esphome_core.yaml AND hardware.yaml
   - `http_request` in system_management.yaml AND esphome_base.yaml
   - `time` in system_management.yaml AND esphome_base.yaml
   - `nimble_base` in ble_improv.yaml AND hardware.yaml
   - These duplicates ensure components are available when needed by other modules

3. **BLE Configuration Consolidated**
   - All BLE sdkconfig options moved to esphome_core.yaml
   - Prevents esp32.framework merge conflicts
   - ALL Bluedroid components explicitly disabled (prevents compilation errors)
   - ble_improv.yaml only contains component definitions

4. **Dependencies Made Clear**
   - Each module header documents its dependencies
   - Import order in Halo-v1.yaml respects dependencies
   - Comments explain what will move in Phase 2

5. **Backward Compatibility Maintained**
   - All existing modules (globals.yaml, pages, weather, Core) still imported
   - No functionality removed
   - System should compile and work identically to before

---

## File Structure

```
packages/
├── system/
│   ├── esphome_core.yaml       # ESP32, PSRAM, BLE config, memory
│   ├── display_hardware.yaml   # Display, touch, I2C, SPI, backlight
│   ├── networking.yaml         # WiFi base (no WireGuard)
│   ├── fonts_colors.yaml       # Fonts, colors, images
│   └── system_management.yaml  # API, time, HTTP, web server
│
├── features/
│   └── ble/
│       └── ble_improv.yaml     # BLE stack, Improv, proxy
│
├── base/ (legacy - Phase 2 will deprecate)
│   ├── globals.yaml
│   ├── hardware.yaml
│   └── esphome_base.yaml
│
├── pages/ (legacy - Phase 2 will reorganize)
│   ├── clock-page.yaml
│   ├── airq-page.yaml
│   └── wifi-page.yaml
│
└── weather/ (legacy - Phase 2 will reorganize)
    ├── weather-led-effects.yaml
    ├── hourly-forecast-pages.yaml
    └── daily-forecast-pages.yaml
```

---

## Dependency Matrix

| Module | Depends On | Optional? |
|--------|-----------|-----------|
| esphome_core | - | ❌ Required |
| fonts_colors | - | ❌ Required |
| display_hardware | esphome_core | ❌ Required |
| networking | esphome_core | ❌ Required |
| system_management | esphome_core | ❌ Required |
| ble_improv | esphome_core, networking | ✅ Yes |

---

## Import Order (CRITICAL)

The import order in Halo-v1.yaml is critical:

1. **esphome_core.yaml** - No dependencies, defines ESP32 config
2. **display_hardware.yaml** - Depends on esphome_core
3. **networking.yaml** - Depends on esphome_core
4. **fonts_colors.yaml** - No dependencies
5. **system_management.yaml** - Depends on esphome_core
6. **ble_improv.yaml** - Depends on esphome_core, networking

Then legacy modules (globals, pages, weather, Core) load after.

---

## What's NOT Changed

- **globals.yaml** - Still contains all globals (Phase 2 will split by capability)
- **hardware.yaml** - Still exists (now deprecated by system modules)
- **esphome_base.yaml** - Still exists (now deprecated by system modules)
- **Halo-v1-Core.yaml** - Still contains all scripts, sensors, LVGL pages (Phase 2 will extract)
- **Page modules** - Still in packages/pages/ (Phase 2 will move to features/)
- **Weather modules** - Still in packages/weather/ (Phase 2 will move to features/)

---

## Testing Status

⚠️ **IN PROGRESS** - Phase 1 structure created, iteratively resolving compilation errors.

### Errors Fixed So Far:

1. ✅ Duplicate `light.monochromatic 'Display Backlight'` - removed from display_hardware.yaml
2. ✅ Duplicate `external_components` - moved to esphome_core.yaml (intentional duplicate documented)
3. ✅ Duplicate `logger` - removed from esphome_core.yaml
4. ✅ Component `nimble_improv` not found - fixed by moving external_components early
5. ✅ Duplicate `binary_sensor 'Display Backlight Active'` - removed from display_hardware.yaml
6. ✅ Duplicate `switch 'Startup Light Blink'` - removed from ble_improv.yaml
7. ✅ `http_request.post` action not found - added http_request to system_management.yaml
8. ✅ WireGuard requires component `time` - added time sources to system_management.yaml
9. ✅ Duplicate global `display_backlight_is_on` - removed display watchdog globals from display_hardware.yaml
10. ✅ Duplicate global `cycleCounter` - removed from ble_improv.yaml, kept in globals.yaml
11. ✅ `weather_helpers` namespace not found - added weather_helpers component to system_management.yaml

### Current Status:

✅ **PHASE 1 COMPLETE!** Successfully compiled after resolving 11 component errors.

### Before proceeding to Phase 2:

1. ✅ Create directory structure
2. ✅ Create all Phase 1 module files
3. ✅ Update Halo-v1.yaml imports
4. ✅ Resolve all duplicate component errors (11 errors fixed)
5. ✅ Test compilation successfully
6. ⏳ Verify all components load correctly (pending hardware test)
7. ⏳ Test on actual hardware (pending deployment)

---

## Next Steps - Phase 2

Phase 2 will extract components from Core and globals into capability modules:

### Planned Extractions:

1. **Clock Capability** (clock_base.yaml + clock_page.yaml)
   - Globals: colon_blink_state, time_update_last_text, page_rotation_vertical_clock_*
   - Scripts: update_colon_widget, time_update
   - Intervals: 2s time update, 1s colon blink
   - Switches: vertical_clock_enabled, time_format, clock_colon_blink
   - Numbers: vertical_clock_order
   - Buttons: go_to_clock

2. **AirQ Capability** (airq_base.yaml + airq_page.yaml)
   - All air quality sensors (SCD4x, MICS4514, BME280, SEN5x)
   - Computed AQI sensor
   - Calibration scripts and buttons
   - Temperature unit selection
   - ~1,200 lines from Core

3. **Weather Capability** (5 files)
   - weather_base.yaml: All weather globals, fetch scripts, sensors
   - weather_page.yaml: Current weather LVGL page
   - weather_daily.yaml: 10-day forecast
   - weather_hourly.yaml: 24-hour forecast
   - weather_hourly_summary.yaml: Summary pages

4. **WiFi Status Capability** (wifi_page_base.yaml + wifi_page.yaml)
   - WiFi page controls
   - WiFi signal sensor
   - Move from packages/pages/

5. **WireGuard Capability** (wireguard.yaml)
   - Extract from Core (lines 8795-8878)
   - Complete VPN configuration
   - WireGuard sensors

6. **Page Rotation Capability** (page_rotation.yaml)
   - Auto-rotation interval and logic
   - Page order management
   - ~700 lines from Core

7. **Diagnostics Capability** (diagnostics.yaml)
   - Heap sensors
   - Performance monitoring
   - ~200 lines from Core

---

## Benefits Achieved So Far

1. **Clear System Organization**
   - System modules clearly separated from features
   - Easy to understand what each module does

2. **Dependency Clarity**
   - Each module documents its dependencies
   - Import order makes sense

3. **Conflict Resolution**
   - BLE config consolidated to prevent merge conflicts
   - No duplicate esp32 configuration blocks

4. **Foundation for Phase 2**
   - Clear structure for capability modules
   - Pattern established for future extractions

---

## Known Issues / Design Compromises

1. **Intentional Duplicates** - Components appear in both new modules and legacy files:
   - `external_components` (esphome_core.yaml + hardware.yaml)
   - `http_request` (system_management.yaml + esphome_base.yaml)
   - `time` (system_management.yaml + esphome_base.yaml)
   - `nimble_base` (ble_improv.yaml + hardware.yaml)
   - **Why:** New modules load first to make components available before legacy files need them
   - **Resolution:** Phase 2 will deprecate legacy files, eliminating duplicates

2. **Split Components** - Some features span multiple files:
   - Backlight: Output in display_hardware.yaml, light component in weather-led-effects.yaml
   - Startup blink: cycleCounter global in ble_improv.yaml, RGB LED in weather-led-effects.yaml, switch in Core
   - Display watchdog: Globals in globals.yaml, interval/button will be added in Phase 2
   - **Why:** Phase 1 avoids breaking existing functionality
   - **Resolution:** Phase 2 will consolidate related components

3. **Minimal Modules** - New modules don't contain full functionality:
   - Most components still in legacy files (globals.yaml, Core)
   - New modules are "placeholders" establishing structure
   - **Why:** Phase 1 focuses on structure, Phase 2 extracts functionality
   - **Resolution:** Phase 2 will extract features from Core into capability modules

---

## Conclusion

Phase 1 has created the capability-based structure foundation. The system modules are now:

- ✅ Clearly organized by purpose (system/ and features/ directories)
- ✅ Well-documented with dependencies and Phase 2 migration notes
- ✅ BLE configuration consolidated to prevent merge conflicts
- 🚧 Iteratively resolving duplicate component errors
- ⏳ Awaiting successful compilation test

**Current State:** Structure in place, most duplicate errors resolved, testing compilation on hardware.

**Next Step:** Complete compilation testing, then proceed with Phase 2 to extract features from the Core file into self-contained capability modules.
