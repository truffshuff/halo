# Halo v1 — ESP32-S3 Air Quality Monitor

An ESPHome-based firmware for the [LilyGo T-Display-Long](https://www.lilygo.cc/products/t-display-long) (180×640 AMOLED, ESP32-S3). Displays real-time air quality, weather, and time on a vertical touchscreen display with Home Assistant integration.

> This is a fork of [yashmulgaonkar/halo](https://github.com/yashmulgaonkar/halo) with significant modular refactoring.

---

## Hardware

| Component | Details |
|-----------|---------|
| Board | LilyGo T-Display-Long (ESP32-S3, 16MB flash, 8MB PSRAM) |
| Display | 180×640 AMOLED (AXS15231 driver) with touchscreen |
| CO2 | SCD40 or SCD41 (I2C) |
| Particulate / VOC / NOx | SEN55 (I2C, address 0x69) |
| Multi-gas (NO2, CO, H2, CH4) | MICS-4514 (I2C, address 0x75) |
| Pressure / Temperature / Humidity | BME280 (I2C) |
| Power management | SY6970 PMIC |
| RGB LED | GPIO47 (optional, used for weather effects) |

---

## Features

- **Air Quality**: CO2, PM1.0/2.5/4.0/10.0, VOC, NOx, multi-gas, US EPA AQI calculation
- **Weather**: Current conditions + 10-day daily + 24-hour hourly forecasts via Home Assistant
- **Clock**: Large vertical time display, 12h/24h, blinking colon
- **WiFi Status**: Signal strength, IP address, SSID, HA connection
- **WireGuard VPN**: Optional encrypted tunnel
- **BLE Provisioning**: WiFi setup via Bluetooth (3 implementation options)
- **Page Rotation**: Automatic cycling through enabled pages
- **LED Effects**: RGB LED driven by weather conditions and AQI
- **OTA Updates**: Over-the-air firmware updates

---

## Quick Start

### Prerequisites

- [ESPHome](https://esphome.io) 2025.11.0 or newer
- Home Assistant with a Long-Lived Access Token
- A weather entity in Home Assistant (e.g. `weather.home`)

### 1. Create `secrets.yaml`

Copy the template and fill in your values. This file is `.gitignore`d and must never be committed.

```yaml
# TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/secrets.yaml

wifi_ssid: "YourSSID"
wifi_password: "YourPassword"

ha_url: "http://homeassistant.local:8123"
ha_token: "your_long_lived_access_token_here"

api_encryption_key: "your_32_byte_base64_key_here"
ota_password: "your_ota_password_here"

# WireGuard (only if wireguard.yaml is enabled)
# wg_address: 10.6.0.2
# wg_prikey: ...
# wg_pubkey: ...
# wg_peerendpt: ...
```

Generate an `api_encryption_key` with: `openssl rand -base64 32`

### 2. Configure `Halo-v1.yaml`

Edit the substitutions at the top of `Halo-v1.yaml` to match your Home Assistant entities:

```yaml
substitutions:
  weather_entity_id: "weather.home"
  current_temp_sensor: "sensor.your_temp_sensor"
  apparent_temp_sensor: "sensor.your_feels_like_sensor"
  wind_speed_sensor: "sensor.your_wind_speed_sensor"
  wind_direction_sensor: "sensor.your_wind_direction_sensor"
```

### 3. Enable/Disable Features

In `Halo-v1.yaml`, comment or uncomment lines in the `packages:` section to toggle features:

```yaml
# BLE — pick exactly one:
# - ...ble/ble_esphome.yaml   # ESPHome native Bluedroid
# - ...ble/ble_improv.yaml    # Custom NimBLE fork (more options)
- ...ble/ble_stub.yaml        # No BLE (saves ~40KB)

# Weather — all optional:
- ...weather/weather_base.yaml          # Required if any weather is enabled
- ...weather/weather_page.yaml          # Current conditions
- ...weather/daily-forecast-pages.yaml  # UI pages for daily forecast
- ...weather/weather_daily.yaml         # 10-day daily forecast data
# - ...weather/weather_hourly.yaml      # 24-hour detailed (largest, ~7KB)
```

### 4. Flash

```bash
esphome run Halo-v1.yaml
```

On first flash use USB. After that, OTA is available.

---

## Project Structure

```
TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/
├── Halo-v1.yaml              # Main entry point (edit this for your setup)
├── Halo-v1-Core.yaml         # Core LVGL init, navigation glue
├── halo-v1-79e384.yaml       # Device-specific override (example)
├── secrets.yaml              # NOT committed — local credentials only
├── fonts/
│   └── materialdesignicons-webfont.ttf
└── packages/
    ├── base/
    │   └── globals.yaml      # Shared global variables (load first)
    ├── system/               # Required system modules
    │   ├── esphome_core.yaml       # ESP32-S3, PSRAM, framework
    │   ├── display_hardware.yaml   # Display driver, I2C/SPI, watchdog
    │   ├── networking.yaml         # WiFi, captive portal
    │   ├── fonts_colors.yaml       # Fonts, color palette (~200KB)
    │   └── system_management.yaml  # API, OTA, time, HTTP, buttons
    └── features/             # Optional feature modules
        ├── ble/              # BLE provisioning (3 variants)
        ├── clock/            # Clock display
        ├── airq/             # Air quality sensors + page
        ├── weather/          # Weather data + pages + LED
        ├── wifi_status/      # WiFi info page
        ├── wireguard/        # WireGuard VPN
        ├── page_rotation/    # Auto page cycling
        └── diagnostics/      # Heap/PSRAM monitoring (dev only)
```

---

## Memory Reference

| Module | RAM Impact |
|--------|-----------|
| System (all required) | ~238KB |
| Fonts & colors | ~200KB |
| BLE (NimBLE) | ~40KB |
| BLE stub (no BLE) | ~0KB |
| Air quality | ~15KB |
| Weather (base + pages) | ~10KB |
| Weather hourly (detailed) | +7KB |
| Clock | ~5KB |
| WiFi status | ~3KB |
| WireGuard | ~8KB |
| Diagnostics | ~5–10KB |
| Page rotation | ~2KB |

---

## Secrets Management

**`secrets.yaml` is excluded from git** via `.gitignore`. It lives only on your local machine and your devices.

- Never commit credentials, tokens, or private keys.
- Use long-lived HA tokens for `ha_token` — revoke and regenerate if ever exposed.
- WireGuard private keys should be treated as sensitive as passwords.

---

## Contributing

This firmware pulls packages from GitHub at build time (`ref: modular`). To develop locally:

1. Change `url:` in `Halo-v1.yaml` to your fork.
2. Change `ref:` to your branch.
3. Or replace `remote_packages:` with `local_packages:` pointing to local paths.

Backup files (`*.bak`, `*.bak[0-9]*`) are excluded by `.gitignore`. Use git history instead.

---

## License

See upstream [yashmulgaonkar/halo](https://github.com/yashmulgaonkar/halo) for license information.
