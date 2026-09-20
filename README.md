# Halo v1 — ESP32-S3 Air Quality Monitor

An ESPHome-based firmware for the [LilyGo T-Display-Long](https://www.lilygo.cc/products/t-display-long) (180×640 AMOLED, ESP32-S3). Displays real-time air quality, weather, and time on a vertical touchscreen display with Home Assistant integration.

> This is a fork of [yashmulgaonkar/halo](https://github.com/yashmulgaonkar/halo) with significant modular refactoring.

**Requires ESPHome 2026.9.0 or newer.**

Further reading:

- [`docs/ARCHITECTURE.md`](TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/docs/ARCHITECTURE.md) — module layout, load order, data flow, display/BLE/network design, known gaps
- [`docs/MEMORY.md`](TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/docs/MEMORY.md) — what lives in internal SRAM vs PSRAM and why; read before touching any buffer size

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
- **3D Printer Status**: BambuLab print progress, temperatures, layers, cover image
- **Diagnostics**: Heap / PSRAM / fragmentation sensors, display watchdog
- **OTA Updates**: Over-the-air firmware updates

---

## Quick Start

### Prerequisites

- [ESPHome](https://esphome.io) **2026.9.0 or newer** (the config uses `network: tcp_send_buffer:`, added in that release)
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

  # Required while the printer packages are enabled (they are, by default).
  # Every printer entity is built as sensor.${printer_device_id}_<suffix> at
  # compile time, so leaving the placeholder in place will not work.
  # If you have no 3D printer, comment out the two printer package lines
  # in the packages: list instead.
  printer_device_id: "your_bambulab_serial"
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
├── Halo-v1.yaml              # TEMPLATE — copy/edit this for your setup
├── Halo-v1-Core.yaml         # Core LVGL init, navigation glue
├── halo-v1-79e384.yaml       # A real working device config (MAC ...79e384) — reference example
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
        ├── printer/          # BambuLab 3D printer status page
        └── diagnostics/      # Heap/PSRAM monitoring (dev only)
```

Documentation lives in `TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/docs/`.

---

## Memory Reference

These are rough per-module costs, **inherited as estimates from the original documentation
and not independently measured** — treat the *location* column as the reliable part and the
sizes as indicative. **Read
[`docs/MEMORY.md`](TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/docs/MEMORY.md) before acting
on them** — the distinction between flash, internal SRAM and PSRAM matters far more than the
totals, and internal SRAM is the only one that is actually scarce.

| Module | Cost | Where |
|--------|------|-------|
| Fonts & colors | ~180KB (est.) | **Flash.** Glyph bitmaps are `const` arrays read in place by LVGL; this is not heap. Three unreferenced icon fonts were removed in 2026.09. |
| LVGL draw buffer | ~115KB | **PSRAM** (at `buffer_size: 50%`). Was ~57.6KB of *internal SRAM* at the old `30%`. |
| BLE (NimBLE) | ~40KB | Mostly **PSRAM** (`CONFIG_BT_NIMBLE_MEM_ALLOC_MODE_EXTERNAL`) |
| BLE stub (no BLE) | ~0KB | — |
| Printer cover image | 64KB download + ~39KB decoded | **PSRAM** (ESPHome `RAMAllocator` defaults to external-first) |
| TCP send/receive buffers | up to 2 × 64KB | **Internal SRAM.** The largest tunable consumer — see MEMORY.md §7. |
| Air quality | ~15KB | code (flash) + small globals |
| Weather (base + pages) | ~10KB | code (flash); forecast JSON parses in **PSRAM** |
| Weather hourly (detailed) | +7KB | code (flash), 8 extra LVGL pages |
| WireGuard | ~8KB | code + session state |
| Printer (code) | ~7KB | code (flash) |
| Diagnostics | ~5–10KB | code (flash) |
| Clock | ~5KB | code (flash) |
| WiFi status | ~3KB | code (flash) |
| AQI history buffer | ~2.3KB | **PSRAM**, allocated lazily |
| Page rotation | ~2KB | code (flash) |

---

## Secrets Management

**`secrets.yaml` is excluded from git** via `.gitignore`. It lives only on your local machine and your devices.

- Never commit credentials, tokens, or private keys.
- Use long-lived HA tokens for `ha_token` — revoke and regenerate if ever exposed.
- WireGuard private keys should be treated as sensitive as passwords.

---

## Contributing

This firmware pulls packages from GitHub at build time (`ref: modular`, `refresh: Always`).

**Editing a file under `packages/` has no effect until it is committed and pushed** — and
because of `refresh: Always`, pushing to `modular` goes live on every device on its next
build. Tag stable points (`git tag v26.09.20`) so devices can pin a known-good `ref:`.

To develop against local files:

1. Change `url:` to your fork and `ref:` to your branch, or
2. Build a local validation harness that swaps `remote_packages:` for local `!include`s —
   the exact recipe is in
   [`docs/ARCHITECTURE.md` §9](TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/docs/ARCHITECTURE.md).

Note that `esphome config` validates structure and IDs but **does not compile lambdas**. Any
change to embedded C++ needs a real `esphome compile` before it is trusted.

Backup files (`*.bak`, `*.bak[0-9]*`) are excluded by `.gitignore`. Use git history instead.

---

## License

See upstream [yashmulgaonkar/halo](https://github.com/yashmulgaonkar/halo) for license information.
