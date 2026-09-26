# Halo v1 — ESP32-S3 Air Quality Monitor

An ESPHome-based firmware for the [LilyGo T-Display-Long](https://www.lilygo.cc/products/t-display-long) (180×640 AMOLED, ESP32-S3). Displays real-time air quality, weather, and time on a vertical touchscreen display with Home Assistant integration.

> This is a fork of [yashmulgaonkar/halo](https://github.com/yashmulgaonkar/halo) with significant modular refactoring.

**Requires ESPHome 2026.9.0 or newer.**

Further reading:

- [`docs/ARCHITECTURE.md`](TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/docs/ARCHITECTURE.md) — module layout, load order, data flow, display/BLE/network design, known gaps
- [`docs/MEMORY.md`](TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/docs/MEMORY.md) — what lives in internal SRAM vs PSRAM and why; read before touching any buffer size
- [truffshuff/esphome-components](https://github.com/truffshuff/esphome-components) — the external components this firmware pins (touch driver, weather helpers, NimBLE stack)

---

## Hardware

| Component | Details |
|-----------|---------|
| Board | LilyGo T-Display-Long (ESP32-S3, 16MB flash, 8MB PSRAM) |
| Display | 180×640 AMOLED (AXS15231, core `mipi_spi`) with capacitive touch |
| CO2 | SCD40 or SCD41 (I2C) |
| Particulate / VOC / NOx | SEN55 (I2C, address 0x69) |
| Multi-gas (NO2, CO, H2, CH4, ethanol, NH3) | MICS-4514 (I2C, address 0x75) |
| Pressure / Temperature / Humidity | BME280 (I2C) |
| Power management | SY6970 PMIC (on the board; not configured by the firmware) |
| RGB LED | 12 × WS2812 on GPIO47 (used for weather and AQI effects) |

---

## Features

- **Air Quality**: CO2, PM1.0/2.5/4.0/10.0, VOC, NOx, multi-gas, US EPA AQI calculation
- **Offline history**: air-quality readings buffered in PSRAM while Home Assistant is unreachable and replayed on reconnect
- **Weather**: Current conditions + 10-day daily + 24-hour hourly forecasts via Home Assistant
- **Clock**: Large vertical time display, 12h/24h, blinking colon, selectable time zone
- **WiFi Status**: Signal strength, IP address, SSID, HA connection, WireGuard status
- **WireGuard VPN**: Optional encrypted tunnel
- **Bluetooth**: Home Assistant Bluetooth proxy and Improv WiFi provisioning (3 package options)
- **Page Rotation**: Automatic cycling through enabled pages
- **LED Effects**: RGB LED driven by weather conditions and AQI
- **3D Printer Status**: BambuLab print progress, temperatures, layers, AMS, cover image
- **Diagnostics**: Heap / PSRAM / fragmentation sensors, display watchdog
- **OTA Updates**: Over-the-air firmware updates

---

## Quick Start

### Prerequisites

- [ESPHome](https://esphome.io) **2026.9.0 or newer** (the config uses `network: tcp_send_buffer:`, added in that release)
- Home Assistant with a Long-Lived Access Token
- A weather entity in Home Assistant (e.g. `weather.home`)

### 1. Create `secrets.yaml`

Copy [`secrets.yaml.example`](TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/secrets.yaml.example)
to `secrets.yaml` in the same directory and fill in your values. `secrets.yaml` is
`.gitignore`d and must never be committed.

The template needs these keys:

| Key | Used for |
|---|---|
| `wifihome_ssid`, `wifihome_password` | WiFi |
| `ha_url`, `ha_token` | HTTP calls to Home Assistant (hourly-forecast fallback, printer cover image) |
| `api_encryption_key` | Native API encryption, which also secures OTA uploads (`openssl rand -base64 32`) |
| `wg_address`, `wg_netmask`, `wg_prikey`, `wg_pubkey`, `wg_shrdkey`, `wg_peerendpt`, `wg_peerport`, `wg_allowed_ips` | WireGuard |

The `wg_*` keys are needed even if you never use WireGuard: the WireGuard package
cannot be switched off yet (see [Known gaps](TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/docs/ARCHITECTURE.md#10-known-gaps)).
Placeholders are fine while the **Enable WireGuard** switch stays off, which is the default.

For `ha_url`, prefer Home Assistant's IP and port (`http://<ip>:8123`) over a
hostname if the device sits on an isolated or IoT VLAN. HTTP requests are
synchronous, and a name that cannot resolve blocks the device for seconds per attempt.

### 2. Configure `Halo-v1.yaml`

Edit the substitutions at the top of `Halo-v1.yaml` to match your Home Assistant entities:

```yaml
substitutions:
  weather_entity_id: "weather.home"
  current_temp_sensor: "sensor.your_temp_sensor"
  apparent_temp_sensor: "sensor.your_feels_like_sensor"
  wind_speed_sensor: "sensor.your_wind_speed_sensor"
  wind_direction_sensor: "sensor.your_wind_direction_sensor"

  # BambuLab entity prefix: every printer entity is built as
  # sensor.${printer_device_id}_<suffix> at compile time.
  # No printer? Leave the placeholder and turn off the
  # "Page Rotation: 3D Printer Page" switch in Home Assistant.
  printer_device_id: "your_bambulab_serial"
```

### 3. Home Assistant setup

Two things on the Home Assistant side are needed for forecasts:

1. **Allow the device to perform Home Assistant actions.** In Settings → Devices &
   services → ESPHome → *your device* → Configure, enable it. The daily forecast is
   fetched with `weather.get_forecasts` through the native API and fails without it.

2. **Create `sensor.hourly_forecast_esp`.** The hourly forecast is pushed to the
   device as the `forecast` attribute of this sensor, trimmed to 24 entries. The full
   forecast is too big for one ESPHome API message (see
   [ARCHITECTURE.md known gap 4](TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/docs/ARCHITECTURE.md#10-known-gaps)).
   Without the sensor, the device falls back to a synchronous HTTP pull of the whole
   forecast (~60 KB), which blocks it for several seconds. A trigger-based template
   sensor along these lines does the job (replace `weather.home` twice):

   ```yaml
   template:
     - triggers:
         - trigger: homeassistant
           event: start
         - trigger: time_pattern
           minutes: /15
         - trigger: event
           event_type: halo_hourly_forecast_refresh
       actions:
         - action: weather.get_forecasts
           target:
             entity_id: weather.home
           data:
             type: hourly
           response_variable: hourly
       sensor:
         - name: hourly_forecast_esp
           unique_id: hourly_forecast_esp
           state: "{{ now().isoformat() }}"
           attributes:
             forecast: "{{ hourly['weather.home'].forecast[:24] | to_json }}"
   ```

   The sensor name must stay `hourly_forecast_esp`: the firmware subscribes to that
   entity id, and a renamed sensor silently delivers nothing. A trigger-based template
   has no state until a trigger fires, so after a template reload fire the
   `halo_hourly_forecast_refresh` event once.

Optional: to backfill Home Assistant's history after an outage, install the AppDaemon
app in [`HomeAssistant/AppDaemon/apps/`](HomeAssistant/AppDaemon/apps/). It is
currently written for one specific unit; see its header.

### 4. Choose features

The `packages:` list in `Halo-v1.yaml` is laid out so features can be commented out,
but **today only two choices are safe**:

```yaml
# BLE — pick exactly one:
# - ...ble/ble_esphome.yaml   # ESPHome native Bluedroid
# - ...ble/ble_improv.yaml    # NimBLE (fork; upstream proxy code, smaller footprint)
- ...ble/ble_stub.yaml        # No BLE (the template default)

# Diagnostics: heap sensors, display watchdog, reboot buttons, rotation master switch
# - ...diagnostics/diagnostics.yaml
```

Every other feature package is referenced by `Halo-v1-Core.yaml`'s page navigation or by
a required module, so commenting it out fails validation with "Couldn't find ID". To
hide a page you don't want, turn off its **Page Rotation: …** switch in Home Assistant.
[ARCHITECTURE.md known gap 2](TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/docs/ARCHITECTURE.md#10-known-gaps)
lists exactly who references what.

### 5. Flash

```bash
esphome run Halo-v1.yaml
```

On first flash use USB. After that, OTA is available.

---

## Project Structure

```
HomeAssistant/
├── AppDaemon/apps/           # Offline-history backfill app (optional)
└── dashbaord.yaml            # Printer dashboard example (sic - filename is misspelled)
TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/
├── Halo-v1.yaml              # TEMPLATE — copy/edit this for your setup
├── Halo-v1-Core.yaml         # LVGL block, page navigation, api: triggers (loaded last)
├── halo-v1-79e384.yaml       # A real working device config (MAC ...79e384) — reference example
├── secrets.yaml.example      # Template for secrets.yaml
├── secrets.yaml              # NOT committed — local credentials only
├── docs/                     # ARCHITECTURE.md, MEMORY.md
├── fonts/
│   └── materialdesignicons-webfont.ttf
└── packages/
    ├── base/
    │   └── globals.yaml      # Shared global variables (load first)
    ├── system/               # Required system modules
    │   ├── esphome_core.yaml       # ESP32-S3, PSRAM, sdkconfig, external components
    │   ├── display_hardware.yaml   # Display, touch, I2C/SPI buses, backlight output
    │   ├── networking.yaml         # WiFi, web server, captive portal, RSSI, startup LED
    │   ├── fonts_colors.yaml       # Fonts, color palette (~180KB flash)
    │   └── system_management.yaml  # HTTP client, time + time zone, reachability diagnostics
    └── features/             # Feature modules
        ├── ble/              # Bluetooth proxy + Improv (3 variants)
        ├── clock/            # Clock display
        ├── airq/             # Air quality sensors, page, offline history
        ├── weather/          # Weather data + pages + LED effects
        ├── wifi_status/      # WiFi info page
        ├── wireguard/        # WireGuard VPN
        ├── page_rotation/    # Auto page cycling
        ├── printer/          # BambuLab 3D printer status page
        └── diagnostics/      # Heap/PSRAM monitoring, display watchdog, reboot buttons
```

---

## Memory Reference

These are rough per-module costs, **mostly inherited as estimates from the original
documentation and not independently measured** — treat the *location* column as the
reliable part and the sizes as indicative. **Read
[`docs/MEMORY.md`](TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/docs/MEMORY.md) before acting
on them** — the distinction between flash, internal SRAM and PSRAM matters far more than the
totals, and internal SRAM is the only one that is actually scarce.

| Module | Cost | Where |
|--------|------|-------|
| Fonts & colors | ~180KB (est.) | **Flash.** Glyph bitmaps are `const` arrays read in place by LVGL; this is not heap. Three unreferenced icon fonts were removed in 2026.09. |
| LVGL draw buffer | ~115KB | **PSRAM** (at `buffer_size: 50%`). Was ~57.6KB of *internal SRAM* at the old `30%`. |
| AQI history buffer | ~270KB at defaults (64 B/sample, up to ~1.26MB) | **PSRAM**, allocated only once HA goes offline; size set from HA number entities |
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
| Time Zone select | +3KB flash, +96 B internal SRAM | measured, ESPHome 2026.9.0 |
| WiFi status | ~3KB | code (flash) |
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
build. Tag stable points (`git tag vYY.MM.DD`, e.g. `v26.09.21`) so devices can pin a
known-good `ref:`.

To develop against local files:

1. Change `url:` to your fork and `ref:` to your branch, or
2. Build a local validation harness that swaps `remote_packages:` for local `!include`s —
   the exact recipe is in
   [`docs/ARCHITECTURE.md` §9](TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/docs/ARCHITECTURE.md).

Note that `esphome config` validates structure and IDs but **does not compile lambdas**. Any
change to embedded C++ needs a real `esphome compile` before it is trusted.

The external components are pinned by commit SHA in two places
(`packages/system/esphome_core.yaml` and `packages/features/ble/ble_improv.yaml`). A change
in [esphome-components](https://github.com/truffshuff/esphome-components) takes effect only
after both pins are moved to the new commit and pushed.

Backup files (`*.bak`, `*.bak[0-9]*`) are excluded by `.gitignore`. Use git history instead.

---

## License

See upstream [yashmulgaonkar/halo](https://github.com/yashmulgaonkar/halo) for license information.
