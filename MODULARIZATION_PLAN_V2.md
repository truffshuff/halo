# Halo v1 Modularization Plan V2 - Capability-Based Organization
**Created:** 2025-11-13
**Updated:** 2025-11-14
**Branch:** modular
**Strategy:** Feature-complete capability modules
**Current Status:** 🚧 Phase 1 In Progress - System module structure created, testing compilation

---

## Current Progress

| Phase | Status | Progress | Details |
|-------|--------|----------|---------|
| **Phase 1** | ✅ Complete | 100% | System modules created, 11 errors fixed, compilation successful! |
| **Phase 2** | 🚧 In Progress | 86% | 6 of 7 capabilities extracted! Core reduced by 1,440 lines. Remaining: Weather (largest, most complex) |
| **Phase 3** | ⏳ Not Started | 0% | Documentation and cleanup |

**Latest Update:** Phase 2 at 86%! Extracted: Clock, AirQ, WiFi Status, WireGuard, Diagnostics, and Page Rotation (6/7 done). Core file reduced from 8,971 to 7,531 lines (-16% reduction). All capabilities are self-contained modules with clear dependencies. Only Weather capability remains (most complex: ~10,500 lines across 5 files). See [PHASE_2_PROGRESS.md](PHASE_2_PROGRESS.md) for details.

---

## Executive Summary

**Goal:** Organize packages by capability/feature so each feature has ALL its components (globals, scripts, sensors, buttons, pages) in one place.

**Philosophy:**
- **Easy Feature Management** - Want to disable AirQ? Remove one module.
- **Clear Dependencies** - Each capability is self-contained with explicit dependencies.
- **User-Friendly** - Non-technical users can understand what each module does.

**Current State:**
- Core file: 8,970 lines
- Existing packages: Mixed organization (foundation, pages by type)
- Phases 1-3: Partial modularization

**Target State:**
- Core file: ~100 lines (imports only)
- Capability modules: 15-20 self-contained feature packages
- Each capability owns its globals, scripts, sensors, controls, pages, buttons

---

## Proposed Capability-Based Structure

```
packages/
├── system/                      # Core system functionality (required)
│   ├── esphome_core.yaml       # Board, PSRAM, ESP32 config, logger
│   ├── display_hardware.yaml   # Display, touchscreen, backlight drivers
│   ├── networking.yaml         # WiFi base config (no WireGuard)
│   ├── fonts_colors.yaml       # Fonts, colors, images (shared resources)
│   └── system_management.yaml  # OTA, reboot, factory reset, API base
│
├── features/
│   ├── clock/
│   │   ├── clock_base.yaml         # Everything for clock capability
│   │   └── clock_page.yaml         # LVGL page (optional: keep separate for size)
│   │
│   ├── airq/
│   │   ├── airq_base.yaml          # Globals, scripts, sensors, controls, buttons
│   │   └── airq_page.yaml          # LVGL page
│   │
│   ├── weather/
│   │   ├── weather_base.yaml       # Globals, scripts, sensors, controls, buttons
│   │   ├── weather_page.yaml       # Main weather LVGL page
│   │   ├── weather_daily.yaml      # Daily forecast pages + update scripts
│   │   ├── weather_hourly.yaml     # Hourly forecast pages + update scripts
│   │   ├── weather_hourly_summary.yaml  # Hourly summary pages + scripts
│   │   └── weather_led_effects.yaml     # LED animations (existing)
│   │
│   ├── wifi_status/
│   │   ├── wifi_page_base.yaml     # Sensors, controls, buttons for WiFi page
│   │   └── wifi_page.yaml          # LVGL page
│   │
│   ├── wireguard/
│   │   └── wireguard.yaml          # Complete WireGuard config, sensors, controls
│   │
│   ├── ble/
│   │   └── ble_improv.yaml         # NimBLE + Improv WiFi provisioning
│   │
│   ├── page_rotation/
│   │   └── page_rotation.yaml      # Auto-rotation globals, intervals, controls
│   │
│   └── diagnostics/
│       └── diagnostics.yaml        # Heap sensors, performance monitoring
│
└── integration/
    └── homeassistant.yaml          # HA-specific integrations, API services
```

---

## Detailed Module Breakdown

### SYSTEM MODULES (Required - 5 files)

#### 1. `packages/system/esphome_core.yaml` (~150 lines)
**Purpose:** Core ESP32 hardware and ESPHome configuration

**Extract from:**
- Current: hardware.yaml (partial)
- Core: ESP32 config, PSRAM, platformio options

**Contents:**
```yaml
# Components:
- substitutions: (device name, version)
- esphome: (name, project, platformio_options)
- esp32: (board, flash_size, framework, sdkconfig)
- psram: (mode, speed)
- logger: (level, logs filter)
```

**Dependencies:** None
**User Benefit:** Can't be disabled (required for boot)

---

#### 2. `packages/system/display_hardware.yaml` (~200 lines)
**Purpose:** Display, touchscreen, SPI/I2C buses, backlight

**Extract from:**
- Current: hardware.yaml (partial)

**Contents:**
```yaml
# Components:
- spi: (QSPI for display)
- i2c: (2 buses - sensors and touch)
- display: (AXS15231 AMOLED)
- touchscreen: (AXS15231 touch)
- output: (backlight PWM)
- light: (backlight control)
- external_components: (axs15231, sy6970, rmt driver)
- binary_sensor: display_backlight_active
- globals: (display watchdog state)
- interval: (30s display watchdog, 10min health check)
- button: recover_display
- switch: display_watchdog
```

**Dependencies:** esphome_core.yaml
**User Benefit:** Display management in one place

---

#### 3. `packages/system/networking.yaml` (~80 lines)
**Purpose:** WiFi base configuration (without WireGuard)

**Extract from:**
- Core: Lines 8777-8828 (WiFi only, not WireGuard)

**Contents:**
```yaml
# Components:
- wifi: (ssid, password, ap, on_connect)
- captive_portal: (for AP mode)
- sensor: wifi_signal
- text_sensor: wifi_ip_address (if not already in ESPHome)
```

**Dependencies:** esphome_core.yaml
**User Benefit:** Basic WiFi separate from VPN

---

#### 4. `packages/system/fonts_colors.yaml` (~320 lines)
**Purpose:** Shared UI resources (fonts, colors, images)

**Extract from:**
- Current: esphome_base.yaml (fonts, colors, images only)

**Contents:**
```yaml
# Components:
- font: (9 Montserrat sizes + 6 Material Design Icon sizes)
- color: (my_red, my_green, my_blue, etc.)
- image: (ym_logo)
```

**Dependencies:** None
**User Benefit:** Shared resources, can customize colors globally

---

#### 5. `packages/system/system_management.yaml` (~150 lines)
**Purpose:** System control, updates, factory reset

**Extract from:**
- Current: esphome_base.yaml (api, time, http_request)
- Core: API services, system buttons

**Contents:**
```yaml
# Components:
- api: (base config, encryption)
- ota: (if not in device YAML)
- time: (homeassistant + sntp fallback)
- http_request: (for weather API)
- button: esp_reboot
- button: factory_reset
- button: clear_improv_wifi
- binary_sensor: online_status
- sensor: uptime
```

**Dependencies:** esphome_core.yaml
**User Benefit:** System management in one place

---

### FEATURE MODULES (Optional - Each is independent)

---

### CLOCK CAPABILITY

#### 6. `packages/features/clock/clock_base.yaml` (~200 lines)
**Purpose:** Everything needed for clock functionality (except LVGL page)

**Extract from:**
- Current: globals.yaml (clock-related globals)
- Core: Clock scripts, switches, numbers

**Contents:**
```yaml
# Globals:
- colon_blink_state
- time_update_last_text
- page_rotation_vertical_clock_enabled
- page_rotation_vertical_clock_order

# Scripts:
- script: update_colon_widget
- script: time_update

# Switches:
- switch: vertical_clock_enabled (page rotation)
- switch: time_format (24h vs 12h)
- switch: clock_colon_blink

# Numbers:
- number: vertical_clock_order (page rotation order)

# Intervals:
- interval: 2s (calls time_update script)
- interval: 1s (updates colon blink)

# Buttons:
- button: go_to_clock (navigation)
```

**Dependencies:**
- system/esphome_core.yaml
- system/fonts_colors.yaml
- system/system_management.yaml (time sources)

**User Benefit:** Disable entire clock feature by removing this + clock_page.yaml

---

#### 7. `packages/features/clock/clock_page.yaml` (~120 lines)
**Purpose:** LVGL clock page display

**Extract from:**
- Current: pages/clock-page.yaml (existing)

**Contents:**
```yaml
# Components:
- lvgl.pages: vertical_clock_page
  - Large time display (hours, colon, minutes, AM/PM)
  - Date and day of week
  - Navigation button
```

**Dependencies:**
- features/clock/clock_base.yaml
- system/fonts_colors.yaml

**User Benefit:** Keep separate from base for clarity (page is large LVGL definition)

---

### AIR QUALITY CAPABILITY

#### 8. `packages/features/airq/airq_base.yaml` (~1,200 lines)
**Purpose:** Everything for air quality monitoring (except LVGL page)

**Extract from:**
- Current: globals.yaml (airq-related globals)
- Core: Air quality sensors, scripts, controls, buttons

**Contents:**
```yaml
# Globals:
- page_rotation_airq_enabled
- page_rotation_airq_order
- temp_unit_changed

# Sensors (6 sensor platforms):
- sensor.computed_aqi
- sensor.scd4x (CO2)
- sensor.mics_4514 (NO2, CO, H2, CH4, Ethanol, NH3)
- sensor.bme280 (Temperature, Humidity, Pressure)
- sensor.sen5x (PM1.0, PM2.5, PM4.0, PM10.0, VOC, NOx, Temp, Humidity)

# Switches:
- switch: airq_page_enabled

# Numbers:
- number: airq_page_order
- number: sen55_temperature_offset
- number: sen55_humidity_offset

# Selects:
- select: display_temperature_unit (F/C/K)

# Buttons:
- button: start_co2_calibration
- button: confirm_co2_calibration
- button: clean_sen55
- button: go_to_airq

# API Services:
- api.service: calibrate_co2_value
- api.service: sen55_clean

# Text Sensors:
- text_sensor: voc_quality (derived from VOC index)
```

**Dependencies:**
- system/esphome_core.yaml
- system/display_hardware.yaml (I2C buses)
- system/fonts_colors.yaml

**User Benefit:**
- Disable if hardware not present
- All air quality config in one place
- Easy to customize sensor thresholds

---

#### 9. `packages/features/airq/airq_page.yaml` (~800 lines)
**Purpose:** LVGL air quality display page

**Extract from:**
- Current: pages/airq-page.yaml (existing)

**Contents:**
```yaml
# Components:
- lvgl.pages: airq_page
  - AQI box with color coding
  - Temperature display
  - CO2 display
  - PM1, PM2.5, PM4, PM10 displays
  - VOC display
  - CO, Ethanol, H2, NO2, NH3, CH4 displays
  - Humidity display
  - Navigation button
```

**Dependencies:**
- features/airq/airq_base.yaml
- system/fonts_colors.yaml

**User Benefit:** Large LVGL page separate from logic

---

### WEATHER CAPABILITY

#### 10. `packages/features/weather/weather_base.yaml` (~800 lines)
**Purpose:** Core weather functionality (globals, fetch scripts, sensors, controls)

**Extract from:**
- Current: globals.yaml (weather globals - ~45 variables)
- Core: Weather fetch scripts, sensors, controls, buttons

**Contents:**
```yaml
# Globals (45+ weather data variables):
- current_temp, current_condition, current_icon
- daily_forecast_temp_high[0-9]
- daily_forecast_temp_low[0-9]
- daily_forecast_condition[0-9]
- hourly_forecast_temp[0-23]
- hourly_forecast_condition[0-23]
- weather_data_valid
- weather_last_fetch_time
- ... (all weather globals)

# Scripts:
- script: fetch_daily_forecast (HTTP call + JSON parse)
- script: fetch_hourly_forecast (HTTP call + JSON parse)
- script: fetch_weather_data (calls both above)
- script: update_weather_icon_color

# Sensors:
- sensor: ha_current_temperature
- sensor: ha_apparent_temperature
- sensor: wind_speed
- sensor: wind_direction

# Switches:
- switch: weather_page_enabled
- switch: daily_forecast_enabled
- switch: hourly_forecast_enabled
- switch: hourly_summary_enabled

# Numbers:
- number: weather_page_order
- number: daily_forecast_order
- number: hourly_forecast_order
- number: hourly_summary_order
- number: weather_gauge_value

# Selects:
- select: led_effect (weather-based)

# Buttons:
- button: refresh_weather
- button: go_to_weather

# Intervals:
- interval: 15min (hourly forecast refresh)
- interval: 60min (daily forecast refresh)
```

**Dependencies:**
- system/esphome_core.yaml
- system/system_management.yaml (http_request, time)
- system/fonts_colors.yaml

**User Benefit:**
- Disable entire weather capability (saves memory)
- All weather data management in one place

---

#### 11. `packages/features/weather/weather_page.yaml` (~960 lines)
**Purpose:** Main weather current conditions LVGL page

**Extract from:**
- Core: Lines 6356-7316 (weather_forecast_page)

**Contents:**
```yaml
# Components:
- lvgl.pages: weather_forecast_page
- script: update_weather_display (updates this page)
```

**Dependencies:**
- features/weather/weather_base.yaml
- system/fonts_colors.yaml

**User Benefit:** Current weather display separate from forecasts

---

#### 12. `packages/features/weather/weather_daily.yaml` (~2,450 lines)
**Purpose:** 10-day daily forecast pages + update scripts

**Extract from:**
- Current: weather/daily-forecast-pages.yaml (818 lines - LVGL pages)
- Core: daily forecast scripts (1,610 lines)

**Contents:**
```yaml
# Scripts:
- script: update_daily_forecast_display (1,040 lines)
- script: update_hourly_summary_page_1 (260 lines)
- script: update_hourly_summary_page_2 (260 lines)

# LVGL Pages:
- lvgl.pages: daily_forecast_page (10-day forecast)

# Buttons:
- button: go_to_daily_forecast
```

**Dependencies:**
- features/weather/weather_base.yaml (globals, fetch scripts)
- system/fonts_colors.yaml

**User Benefit:**
- Can disable daily forecast (saves ~2.5KB)
- Keep pages with their update scripts

---

#### 13. `packages/features/weather/weather_hourly.yaml` (~7,100 lines)
**Purpose:** 24-hour hourly forecast pages + update scripts

**Extract from:**
- Current: weather/hourly-forecast-pages.yaml (5,476 lines - LVGL pages)
- Core: hourly update scripts (1,600 lines)

**Contents:**
```yaml
# Scripts (8 scripts, each updates 3-hour page):
- script: update_hourly_page_1
- script: update_hourly_page_2
- script: update_hourly_page_3
- script: update_hourly_page_4
- script: update_hourly_page_5
- script: update_hourly_page_6
- script: update_hourly_page_7
- script: update_hourly_page_8

# LVGL Pages (8 pages):
- lvgl.pages: hourly_page_1 through hourly_page_8

# Buttons:
- button: go_to_hourly_forecast
```

**Dependencies:**
- features/weather/weather_base.yaml (globals, fetch scripts)
- system/fonts_colors.yaml

**User Benefit:**
- Can disable hourly forecast (saves ~7KB!)
- Largest memory savings potential

---

#### 14. `packages/features/weather/weather_hourly_summary.yaml` (~1,200 lines)
**Purpose:** Hourly summary pages (2 pages with 12 hours each)

**Extract from:**
- Current: weather/hourly-forecast-pages.yaml (partial - page 1 only?)
- Core: Scripts and page 2

**Contents:**
```yaml
# LVGL Pages:
- lvgl.pages: hourly_summary_page_1 (12 hours)
- lvgl.pages: hourly_summary_page_2 (12 hours)

# Buttons:
- button: go_to_hourly_summary
```

**Dependencies:**
- features/weather/weather_base.yaml
- features/weather/weather_daily.yaml (update scripts)
- system/fonts_colors.yaml

**User Benefit:** Optional summary view

---

#### 15. `packages/features/weather/weather_led_effects.yaml` (~264 lines)
**Purpose:** Weather-based RGB LED animations

**Extract from:**
- Current: weather/weather-led-effects.yaml (existing)

**Contents:**
```yaml
# Components:
- light: rgb_light (LED strip)
- light.effects: (12 weather effects)
- script: (effect activation logic)
```

**Dependencies:**
- features/weather/weather_base.yaml (globals for condition)
- system/display_hardware.yaml (GPIO47 pin)

**User Benefit:** Optional LED feature

---

### WIFI STATUS PAGE CAPABILITY

#### 16. `packages/features/wifi_status/wifi_page_base.yaml` (~70 lines)
**Purpose:** WiFi status page controls and sensors (no WireGuard)

**Extract from:**
- Current: globals.yaml (wifi page globals)
- Core: WiFi page controls

**Contents:**
```yaml
# Globals:
- page_rotation_wifi_enabled
- page_rotation_wifi_order

# Switches:
- switch: wifi_page_enabled

# Numbers:
- number: wifi_page_order

# Buttons:
- button: go_to_wifi

# Note: WiFi sensor already in system/networking.yaml
```

**Dependencies:**
- system/networking.yaml

**User Benefit:** Can disable WiFi status page

---

#### 17. `packages/features/wifi_status/wifi_page.yaml` (~180 lines)
**Purpose:** WiFi status LVGL page

**Extract from:**
- Current: pages/wifi-page.yaml (existing)

**Contents:**
```yaml
# Components:
- lvgl.pages: wifi_page
  - WiFi signal strength
  - IP address
  - SSID
  - Home Assistant connection status
  - Navigation button
```

**Dependencies:**
- features/wifi_status/wifi_page_base.yaml
- system/fonts_colors.yaml

**User Benefit:** Optional WiFi monitoring page

---

### WIREGUARD CAPABILITY

#### 18. `packages/features/wireguard/wireguard.yaml` (~150 lines)
**Purpose:** Complete WireGuard VPN feature

**Extract from:**
- Core: Lines 8795-8878 (WireGuard config)
- Core: WireGuard sensors

**Contents:**
```yaml
# WireGuard Configuration:
- wireguard: (id, address, netmask, keys, peer config)

# Sensors:
- sensor: wireguard_status
- sensor: wireguard_address
- text_sensor: wireguard_ip
- text_sensor: wireguard_endpoint
- text_sensor: wireguard_latest_handshake
- binary_sensor: wireguard_connected

# Switches:
- switch: enable_wireguard

# NOTE: WireGuard UI elements in wifi_page.yaml
```

**Dependencies:**
- system/networking.yaml (WiFi must work first)

**User Benefit:**
- Completely optional feature
- Remove = no VPN functionality
- WireGuard netmask comment preserved here

---

### BLE CAPABILITY

#### 19. `packages/features/ble/ble_improv.yaml` (~120 lines)
**Purpose:** NimBLE stack + Improv WiFi provisioning

**Extract from:**
- Current: hardware.yaml (NimBLE config)

**Contents:**
```yaml
# ESP32 BLE Configuration:
- esp32.framework.sdkconfig_options:
    CONFIG_BT_ENABLED: "y"
    CONFIG_BT_BLUEDROID_ENABLED: "n"
    CONFIG_BT_NIMBLE_ENABLED: "y"
    CONFIG_BT_NIMBLE_MAX_CONNECTIONS: 1
    (... all NimBLE flags)

# BLE Components:
- esp32_ble: (configuration)
- nimble_improv: (WiFi provisioning)

# Intervals:
- interval: 1s (startup blink - 2 intervals for RGB LED during provisioning)

# Switches:
- switch: startup_light_blink

# Globals:
- cycleCounter (for blink timing)
```

**Dependencies:**
- system/esphome_core.yaml (PSRAM needed for BLE)
- system/networking.yaml (provisions WiFi)
- features/weather/weather_led_effects.yaml (RGB LED, but could be optional)

**User Benefit:**
- Disable BLE to save memory (~40KB)
- Keep if you need WiFi provisioning via phone

---

### PAGE ROTATION CAPABILITY

#### 20. `packages/features/page_rotation/page_rotation.yaml` (~700 lines)
**Purpose:** Auto page rotation system

**Extract from:**
- Current: globals.yaml (rotation globals)
- Core: Auto-rotation interval (largest interval, ~500 lines)

**Contents:**
```yaml
# Globals:
- current_page_name
- last_auto_rotation_time
- auto_page_rotation_enabled
- auto_page_rotation_interval
- page_order_array (computed from individual page orders)

# Scripts:
- script: page_transition_cleanup

# Switches:
- switch: auto_page_rotation

# Numbers:
- number: auto_page_rotation_interval

# Intervals:
- interval: 1s (main auto-rotation state machine - COMPLEX)
```

**Dependencies:**
- ALL page modules (clock, airq, wifi, weather, etc.)
- system/fonts_colors.yaml

**User Benefit:**
- Disable auto-rotation (keeps manual navigation)
- Most complex interval isolated here

---

### DIAGNOSTICS CAPABILITY

#### 21. `packages/features/diagnostics/diagnostics.yaml` (~200 lines)
**Purpose:** Performance monitoring and debugging sensors

**Extract from:**
- Core: Diagnostic sensors

**Contents:**
```yaml
# Sensors:
- sensor: display_refresh_count
- sensor: time_update_duration
- sensor: weather_icon_update_duration
- sensor: heap_size
- sensor: heap_free
- sensor: heap_max_alloc
- sensor: heap_min_free
- sensor: heap_fragmentation
- sensor: psram_free
- sensor: psram_size
- sensor: display_last_update
- sensor: display_recovery_count

# Globals (if any diagnostic state):
- (performance tracking globals)
```

**Dependencies:**
- system/esphome_core.yaml

**User Benefit:**
- Disable to save memory (~5-10KB)
- Keep for debugging performance issues

---

### HOME ASSISTANT INTEGRATION

#### 22. `packages/integration/homeassistant.yaml` (~50 lines)
**Purpose:** Home Assistant-specific integrations

**Extract from:**
- Core: HA-specific entities, services

**Contents:**
```yaml
# Components:
- (Any HA-specific binary sensors, sensors, services)
- (Voice assistant integration if added)
- (HA dashboard cards metadata)
```

**Dependencies:**
- system/system_management.yaml (API)

**User Benefit:**
- Optional HA-specific features
- Easy to add voice assistant later

---

## Core File After Modularization (~100 lines)

### `Halo-v1-Core.yaml` (Configuration Manifest)

```yaml
# ============================================================================
# Halo v1 - Core Configuration Manifest
# ============================================================================
# This file imports feature capability modules.
# Each module is self-contained and can be enabled/disabled.
# Device-specific files (e.g., halo-v1-79e384.yaml) extend this.
# ============================================================================

substitutions:
  name: halo
  version: "25.11.13.1"
  device_description: ${name} - v${version}

# ============================================================================
# SYSTEM MODULES (Required)
# ============================================================================
# These modules are essential for basic operation.
# Do not disable unless you know what you're doing.

packages:
  system:
    url: https://github.com/truffshuff/halo/
    ref: modular
    files:
      # Core Hardware & Software
      - halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/system/esphome_core.yaml
      - halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/system/display_hardware.yaml
      - halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/system/networking.yaml
      - halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/system/fonts_colors.yaml
      - halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/system/system_management.yaml

# ============================================================================
# FEATURE MODULES (Optional)
# ============================================================================
# Each feature is self-contained. Comment out to disable a feature.
# IMPORTANT: Removing a feature saves memory but removes that capability.

  features:
    url: https://github.com/truffshuff/halo/
    ref: modular
    files:
      # -------------------------
      # Clock Feature
      # -------------------------
      # Provides: Large vertical time display, 12h/24h format
      # Memory: ~5KB
      # Hardware: None (uses system time)
      - halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/features/clock/clock_base.yaml
      - halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/features/clock/clock_page.yaml

      # -------------------------
      # Air Quality Monitoring
      # -------------------------
      # Provides: CO2, PM2.5, VOC, Temperature, Humidity monitoring
      # Memory: ~15KB
      # Hardware Required: SCD40, SEN55, MICS4514, BME280 sensors
      - halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/features/airq/airq_base.yaml
      - halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/features/airq/airq_page.yaml

      # -------------------------
      # Weather Information
      # -------------------------
      # Provides: Current conditions, forecasts, LED effects
      # Memory: Base ~8KB, +2KB per forecast module
      # Hardware: RGB LED (optional for effects)
      # Requirements: Home Assistant with weather integration

      # Weather Base (REQUIRED for any weather features)
      - halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/features/weather/weather_base.yaml

      # Weather Pages (Optional - pick what you need)
      - halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/features/weather/weather_page.yaml
      # - halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/features/weather/weather_daily.yaml
      # - halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/features/weather/weather_hourly.yaml
      # - halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/features/weather/weather_hourly_summary.yaml

      # Weather LED Effects (Optional)
      - halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/features/weather/weather_led_effects.yaml

      # -------------------------
      # WiFi Status Page
      # -------------------------
      # Provides: WiFi signal, IP, connection status display
      # Memory: ~3KB
      # Hardware: None (uses built-in WiFi)
      - halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/features/wifi_status/wifi_page_base.yaml
      - halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/features/wifi_status/wifi_page.yaml

      # -------------------------
      # WireGuard VPN (Optional)
      # -------------------------
      # Provides: Encrypted VPN tunnel to home network
      # Memory: ~8KB
      # Hardware: None
      # Requirements: WireGuard server configured
      # - halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/features/wireguard/wireguard.yaml

      # -------------------------
      # BLE WiFi Provisioning
      # -------------------------
      # Provides: Setup WiFi via phone app (Improv)
      # Memory: ~40KB (BLE stack)
      # Hardware: BLE built into ESP32-S3
      - halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/features/ble/ble_improv.yaml

      # -------------------------
      # Page Rotation
      # -------------------------
      # Provides: Automatic page rotation
      # Memory: ~2KB
      # Hardware: None
      - halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/features/page_rotation/page_rotation.yaml

      # -------------------------
      # Diagnostics (Optional)
      # -------------------------
      # Provides: Heap monitoring, performance metrics
      # Memory: ~5KB
      # Hardware: None
      # Useful for: Debugging, development
      # - halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/features/diagnostics/diagnostics.yaml

# ============================================================================
# INTEGRATION MODULES (Optional)
# ============================================================================
  integration:
    url: https://github.com/truffshuff/halo/
    ref: modular
    files:
      # Home Assistant specific integrations
      # - halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/integration/homeassistant.yaml

    refresh: Always
```

---

## Migration Strategy

### Phase 1: Reorganize Existing Files (STATUS: 🚧 IN PROGRESS)

**Goal:** Restructure current 9 packages into capability-based organization without breaking anything.

**Actual Time:** ~2-3 hours (iterative error resolution)

#### Step 1.1: Create New Directory Structure ✅ COMPLETE
```bash
mkdir -p packages/system
mkdir -p packages/features/ble
# Other feature directories will be created in Phase 2
```

#### Step 1.2: Create System Module Files ✅ COMPLETE
Created 5 new system modules:
- ✅ `system/esphome_core.yaml` - ESP32, PSRAM, BLE sdkconfig, external_components
- ✅ `system/display_hardware.yaml` - Display, touchscreen, I2C, SPI, backlight output
- ✅ `system/fonts_colors.yaml` - Fonts, colors, images (copied from esphome_base.yaml)
- ✅ `system/system_management.yaml` - http_request, time sources
- ✅ `system/networking.yaml` - WiFi config with startup blink handlers

#### Step 1.3: Create BLE Feature Module ✅ COMPLETE
- ✅ `features/ble/ble_improv.yaml` - nimble_base, nimble_improv, bluetooth_proxy, cycleCounter

#### Step 1.4: Update Imports ✅ COMPLETE
- ✅ Updated `Halo-v1.yaml` with new package imports
- ✅ New modules load BEFORE legacy modules (critical for dependencies)
- ✅ Added Phase 1 status comments

#### Step 1.5: Resolve Duplicate Component Errors 🚧 IN PROGRESS
Fixed 9 duplicate component errors:
- ✅ Duplicate light 'Display Backlight' - removed from display_hardware.yaml
- ✅ Duplicate external_components - consolidated in esphome_core.yaml (intentional duplicate)
- ✅ Duplicate logger - removed from esphome_core.yaml
- ✅ Component nimble_improv not found - fixed by early external_components loading
- ✅ Duplicate binary_sensor 'Display Backlight Active' - removed from display_hardware.yaml
- ✅ Duplicate switch 'Startup Light Blink' - removed from ble_improv.yaml
- ✅ http_request.post action not found - added to system_management.yaml
- ✅ WireGuard requires time - added time sources to system_management.yaml
- ✅ Duplicate global 'display_backlight_is_on' - removed display watchdog globals
- ✅ Duplicate global 'cycleCounter' - removed from ble_improv.yaml
- ✅ weather_helpers namespace not found - added component to system_management.yaml

#### Step 1.6: Test Compilation ⏳ IN PROGRESS
- ⏳ Testing build through Home Assistant on actual hardware
- ⏳ Awaiting build results after weather_helpers fix

**Key Learning:** Phase 1 modules are "minimal placeholders" that:
- Only include components needed BEFORE legacy files load
- Accept intentional duplicates for proper load order
- Document what will move in Phase 2
- Don't break existing functionality

**Validation:** Compile successfully with new structure (in progress)

---

### Phase 2: Extract Core Components (6-8 hours)

**Goal:** Move remaining components from Core into capability modules.

#### Step 2.1: Clock Capability (1 hour)
1. Create `features/clock/clock_base.yaml`
2. Extract from Core:
   - Scripts: `update_colon_widget`, `time_update`
   - Intervals: 2s time update, 1s colon blink
   - Switches: time_format, clock_colon_blink, vertical_clock_enabled
   - Numbers: vertical_clock_order
   - Buttons: go_to_clock
3. Add clock globals from old globals.yaml
4. Test: Verify clock page displays and updates

#### Step 2.2: Air Quality Capability (2 hours)
1. Create `features/airq/airq_base.yaml`
2. Extract from Core:
   - All sensor platforms: SCD4x, MICS4514, BME280, SEN5x
   - Computed AQI sensor
   - Text sensor: VOC quality
   - API services: calibrate_co2, sen55_clean
   - Buttons: CO2 calibration (2), clean_sen55, go_to_airq
   - Switches: airq_page_enabled
   - Numbers: airq_page_order, sen55 offsets
   - Select: display_temperature_unit
3. Add airq globals
4. Test: Verify all sensors update, buttons work

#### Step 2.3: Weather Capability (3-4 hours)
1. Create `features/weather/weather_base.yaml`
2. Extract from Core:
   - All 45+ weather globals
   - Scripts: fetch_daily, fetch_hourly, fetch_weather_data, update_weather_icon_color
   - Sensors: HA temperature, apparent temp, wind
   - Intervals: 15min hourly refresh, 60min daily refresh
   - Switches: weather/daily/hourly/summary enabled
   - Numbers: weather/daily/hourly/summary order, gauge value
   - Select: led_effect
   - Buttons: refresh_weather, go_to_weather

3. Create `features/weather/weather_page.yaml`
   - Extract weather_forecast_page LVGL
   - Extract update_weather_display script

4. Create `features/weather/weather_daily.yaml`
   - Move daily-forecast-pages.yaml here
   - Extract update_daily_forecast_display script
   - Extract hourly_summary update scripts
   - Buttons: go_to_daily_forecast

5. Create `features/weather/weather_hourly.yaml`
   - Move hourly-forecast-pages.yaml here
   - Extract 8 update_hourly_page scripts
   - Buttons: go_to_hourly_forecast

6. Create `features/weather/weather_hourly_summary.yaml`
   - Extract hourly_summary_page_1 and _2 LVGL
   - Buttons: go_to_hourly_summary

7. Move `weather-led-effects.yaml` to `features/weather/`

Test: Verify weather fetches, pages update, LED effects work

#### Step 2.4: WiFi Status Capability (30 min)
1. Create `features/wifi_status/wifi_page_base.yaml`
2. Extract from Core:
   - Switches: wifi_page_enabled
   - Numbers: wifi_page_order
   - Buttons: go_to_wifi
3. Move wifi-page.yaml to features/wifi_status/
4. Test: WiFi page displays, navigation works

#### Step 2.5: WireGuard Capability (30 min)
1. Create `features/wireguard/wireguard.yaml`
2. Extract from Core:
   - WireGuard config (with netmask comments!)
   - WireGuard sensors (all text/binary/sensor)
   - Switch: enable_wireguard
3. Test: VPN connects, sensors update

#### Step 2.6: BLE Capability (30 min)
1. Create `features/ble/ble_improv.yaml`
2. Move from display_hardware.yaml:
   - NimBLE sdkconfig options
   - esp32_ble component
   - nimble_improv component
3. Extract from Core:
   - Intervals: 1s startup blink (2 intervals)
   - Switch: startup_light_blink
   - Global: cycleCounter
4. Test: Improv provisioning works

#### Step 2.7: Page Rotation Capability (1 hour)
1. Create `features/page_rotation/page_rotation.yaml`
2. Extract from Core:
   - Global: current_page_name, last_auto_rotation_time
   - Script: page_transition_cleanup
   - Interval: 1s auto-rotation (LARGE, ~500 lines)
   - Switch: auto_page_rotation
   - Number: auto_page_rotation_interval
3. Add rotation globals
4. Test: Auto-rotation cycles through enabled pages

#### Step 2.8: Diagnostics Capability (30 min)
1. Create `features/diagnostics/diagnostics.yaml`
2. Extract from Core:
   - All heap sensors (7 sensors)
   - Performance sensors (display refresh, update durations)
   - Recovery count sensor
3. Test: Sensors update in HA

#### Step 2.9: System Modules (1 hour)
1. Create `system/networking.yaml`
   - Extract WiFi config from Core
   - WiFi signal sensor
2. Create `system/system_management.yaml`
   - Move API, time, http_request from esphome_base
   - Extract buttons: reboot, factory_reset, clear_improv_wifi
   - Extract sensors: uptime, online_status
3. Update display_hardware.yaml
   - Add display watchdog interval
   - Add recover_display button
   - Add display_watchdog switch
4. Test: System boots, API connects, buttons work

**Validation:** All features work, Core file is ~100 lines

---

### Phase 3: Update Documentation (1 hour)

1. Update Core file with:
   - Feature descriptions
   - Memory usage per feature
   - Hardware requirements
   - Comment out optional features by default

2. Create README in each capability folder:
   - What the capability does
   - Hardware requirements
   - Dependencies
   - Configuration options

3. Update main device YAML files:
   - Update import paths
   - Add comments explaining features

---

## Dependency Matrix

| Capability | Depends On | Optional? |
|------------|-----------|-----------|
| esphome_core | - | ❌ Required |
| display_hardware | esphome_core | ❌ Required |
| networking | esphome_core | ❌ Required |
| fonts_colors | - | ❌ Required |
| system_management | esphome_core | ❌ Required |
| clock | system_management, fonts_colors | ✅ Yes |
| airq | display_hardware, fonts_colors | ✅ Yes |
| weather | system_management, fonts_colors | ✅ Yes |
| weather_page | weather_base | ✅ Yes |
| weather_daily | weather_base | ✅ Yes |
| weather_hourly | weather_base | ✅ Yes |
| weather_hourly_summary | weather_base, weather_daily | ✅ Yes |
| weather_led_effects | weather_base, display_hardware | ✅ Yes |
| wifi_status | networking, fonts_colors | ✅ Yes |
| wireguard | networking | ✅ Yes |
| ble_improv | esphome_core, networking | ✅ Yes |
| page_rotation | ALL page modules | ✅ Yes |
| diagnostics | esphome_core | ✅ Yes |

---

## Import Order (CRITICAL)

```yaml
packages:
  system:
    files:
      # 1. Core (no dependencies)
      - system/esphome_core.yaml
      - system/fonts_colors.yaml

      # 2. Base infrastructure
      - system/networking.yaml
      - system/system_management.yaml
      - system/display_hardware.yaml

  features:
    files:
      # 3. Capabilities (order doesn't matter within this group)
      - features/clock/clock_base.yaml
      - features/airq/airq_base.yaml
      - features/weather/weather_base.yaml
      - features/wifi_status/wifi_page_base.yaml
      - features/wireguard/wireguard.yaml
      - features/ble/ble_improv.yaml
      - features/diagnostics/diagnostics.yaml

      # 4. Pages (after base capabilities)
      - features/clock/clock_page.yaml
      - features/airq/airq_page.yaml
      - features/weather/weather_page.yaml
      - features/weather/weather_daily.yaml
      - features/weather/weather_hourly.yaml
      - features/weather/weather_hourly_summary.yaml
      - features/weather/weather_led_effects.yaml
      - features/wifi_status/wifi_page.yaml

      # 5. Page rotation (MUST BE LAST - depends on all pages)
      - features/page_rotation/page_rotation.yaml
```

---

## Benefits of Capability-Based Organization

### For Users:
1. **Easy Feature Management**
   - Want to disable weather? Comment out 1-6 lines
   - Want to disable WireGuard? Comment out 1 line
   - Want minimal device? Keep only clock + airq

2. **Clear Memory Trade-offs**
   - Each feature documents its memory cost
   - Easy to see what's using memory
   - Can build "lite" versions for low-memory scenarios

3. **Hardware Flexibility**
   - Missing SEN55? Comment out airq capability
   - No RGB LED? Comment out weather_led_effects
   - No BLE needed? Comment out ble_improv (saves 40KB!)

### For Developers:
1. **Logical Organization**
   - All clock code in clock/ folder
   - All weather code in weather/ folder
   - Easy to find what you're looking for

2. **Clear Dependencies**
   - Each file documents what it depends on
   - Can't accidentally break dependencies (compile fails)
   - Easy to add new capabilities

3. **Easier Testing**
   - Test one capability at a time
   - Isolate issues to specific feature
   - Can disable problematic features temporarily

### For Maintainers:
1. **Modular Updates**
   - Update weather without touching clock
   - Add new sensors without changing pages
   - Refactor one capability without affecting others

2. **Code Reuse**
   - Other projects can import specific capabilities
   - Share weather module with other ESPHome displays
   - Create "Halo Lite" with subset of features

---

## Next Steps

**Ready to proceed?**

### Questions:
1. **Does this capability-based structure make sense to you?**
2. **Any capabilities you'd organize differently?**
3. **Should we start with Phase 1 (reorganizing existing files)?**
4. **Or jump to Phase 2 (extracting from Core)?**
5. **Do you want WireGuard and diagnostics enabled by default or commented out?**

Let me know your thoughts and we can begin the migration!
