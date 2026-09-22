# Halo v1 — Architecture

How this firmware is put together, for someone opening the repository for the first time.

Companion document: [MEMORY.md](MEMORY.md) explains *why* memory is placed where it is.
Read it before changing any buffer size, `sdkconfig_option`, or allocation.

Baseline: **ESPHome 2026.9.0**, ESP-IDF via the native `esp-idf` toolchain.

---

## 1. What this is

An ESPHome YAML firmware. There is **no compiled application source in this repository** —
every behaviour is expressed as YAML plus C++ lambdas embedded in that YAML. The only
external C++ lives in a separate repository, `truffshuff/esphome-components`, pinned by
commit hash (see §6).

Practical consequence: `esphome config` validates structure and IDs, but **does not compile
the lambdas**. A YAML-valid configuration can still fail to build. Any change to a lambda
needs a real `esphome compile`.

---

## 2. Hardware platform

| Item | Value |
|---|---|
| Board | LilyGo T-Display-Long |
| SoC | ESP32-S3 (dual Xtensa LX7, 240 MHz) |
| Flash | 16 MB, QIO @ 80 MHz |
| PSRAM | 8 MB octal @ 80 MHz |
| Internal SRAM | ~512 KB total; the portion reaching the heap after the WiFi/BLE stacks load is smaller — read it from the `Free Heap` diagnostic sensor rather than assuming a figure |
| Display | 180 × 640 AMOLED, AXS15231, **quad SPI** |
| Touch | AXS15231 capacitive, interrupt-driven on GPIO11 |
| PMIC | SY6970 |
| RGB LED | 12 × WS2812 on GPIO47, RMT driver, DMA enabled |

**Internal SRAM is the scarce resource. PSRAM is not.** That single fact drives most of the
configuration decisions in this project.

### Pin map

| Function | Pins |
|---|---|
| Display QSPI | CLK 17, data 13/18/21/14, CS 12, RESET 16 |
| Display backlight | GPIO1 (plain GPIO on/off — see note) |
| Sensor I²C (`lily_i2c`) | SDA 9, SCL 48 |
| Touch I²C (`touch_i2c`) | SDA 15, SCL 10 @ 400 kHz |
| Touch interrupt | GPIO11 |
| RGB LED | GPIO47 |

> Backlight note: the backlight is a plain GPIO output, not LEDC/PWM. The ESP32-S3 LEDC path
> on IDF 5.5 was crashing on zero-duty updates during boot, and this design only ever needs
> on/off. Do not "upgrade" it to PWM without retesting boot.

---

## 3. Module layout and load order

Configuration is assembled from packages pulled **from GitHub at build time**, not from the
working tree:

```yaml
packages:
  remote_packages:
    url: https://github.com/truffshuff/halo/
    ref: modular
    refresh: Always
```

**This is the single most surprising thing about the repository.** Editing a file under
`packages/` changes nothing until it is committed and pushed to `modular`. And because
`refresh: Always` is set, a push is live on every device on its next build. See §9 for how
to validate local edits before pushing.

Load order is significant — ESPHome merges packages in list order, and an `id:` must be
defined before it is referenced:

```
packages/base/globals.yaml          shared globals — MUST be first
        ↓
packages/system/*                   required; the device does not boot without these
  esphome_core.yaml                 ESP32-S3, PSRAM, api encryption, network:, sdkconfig
  display_hardware.yaml             SPI/I²C buses, display, touch, backlight
  networking.yaml                   wifi:, web_server, captive_portal, RSSI sensor
  fonts_colors.yaml                 fonts, colors, images  (flash cost, not RAM)
  system_management.yaml            http_request, time, weather_helpers, status sensors
        ↓
packages/features/*                 optional; each is meant to be commentable
        ↓
Halo-v1-Core.yaml                   LVGL config, page navigation, API triggers — LAST
```

### Entry points

| File | Role |
|---|---|
| `Halo-v1.yaml` | **Template.** The starting point a new user copies and edits, and what `dashboard_import` adopts. Substitutions here are placeholders meant to be replaced. |
| `halo-v1-<mac>.yaml` | **A real, working device configuration**, named for the last six hex digits of that unit's MAC. `halo-v1-79e384.yaml` is the reference example — when the template and this file disagree, this file is the one that is actually running. |
| `Halo-v1-Core.yaml` | Cross-cutting glue. Not an entry point — it is the last package. |

The two differ on purpose: the template ships BLE stubbed and diagnostics off so it builds
small and without a BLE fork; the working device enables NimBLE and diagnostics.

Because the template is what people start from, it should still **validate as shipped** even
though its values are placeholders. Two defects that broke exactly that were fixed in 2026.09:
`printer_device_id` was the only substitution in the file with no value at all (every other one
carries a concrete example), and `wifi_signal_db` was defined only in the optional diagnostics
package that the template has commented out.

---

## 4. Data flow

```
 I²C sensors ──► ESPHome sensor platforms ──► on_value lambda
   SCD40                                        ├─► lvgl.label.update   (screen)
   SEN55                                        └─► API state           (Home Assistant)
   MICS-4514
   BME280

 Home Assistant ──► homeassistant sensor/text_sensor platforms ──► globals ──► LVGL
   weather.*            (push, over the API connection)
   sensor.<printer>_*

 Home Assistant ──► http_request POST /api/services/weather/get_forecasts
   (pull, ~20 KB JSON, parsed into fixed-size global arrays — see §5)

 Home Assistant ──► image: platform: online_image (printer cover, 180 s poll)
```

Two distinct paths reach Home Assistant, and the difference matters:

- **Native API** (port 6053, Noise-encrypted) — push both ways, used for all entity state.
- **HTTP REST** (`${ha_url}` + `${ha_token}`) — pull only, used for forecast fetches and the
  printer cover image, because `homeassistant.action` with `capture_response` proved
  unreliable for the forecast payload. The relevant weather scripts go straight to HTTP.

---

## 5. Display architecture

LVGL v9 (ESPHome 2026.4.0+) drives an `mipi_spi` display.

- **Pages** are declared across many package files, all merging into one `lvgl:` block.
- **Navigation** is a two-stage loop in `Halo-v1-Core.yaml`:
  1. A **1 s interval** decides *whether* to rotate. It builds the enabled-page list, sorts
     it by the user's per-page order number, and writes the next page name into the
     `current_page_name` global.
  2. A **100 ms interval** notices `current_page_name` changed and issues the matching
     `lvgl.page.show`.

  The split exists because `lvgl.page.show` needs a literal page id — it cannot take a
  lambda — so the second stage is a nested if/else chain over the page name. It is verbose
  but it is not accidental.

- **Rotation order ties**: as of 2026.09 the sort is a stable insertion sort. If two pages
  are given the *same* order number, they now rotate in declaration order (clock, airq, wifi,
  weather, hourly, hourly-summary, daily, printer). The previous selection sort was unstable
  and the resulting order was arbitrary. Shipped defaults are 1–8 with no ties, so this only
  shows up if a user sets two pages to the same number in Home Assistant.

- **Rendering discipline**: every widget that updates frequently has a paired
  `*_last_text` / `*_needs_render` global. The rule is: compute the new string, compare, and
  only call `lvgl.label.update` when it changed **and** its page is currently visible.
  Follow this pattern for new widgets — the display is on a 5 MHz quad-SPI link and
  redundant repaints are expensive.

- **Watchdog**: `features/diagnostics/diagnostics.yaml` watches `last_display_update_time`
  and forces a hardware recovery if flushes stop. Only active when diagnostics is enabled.

---

## 6. External components

Declared in two places, deliberately:

| Where | Components | Why |
|---|---|---|
| `system/esphome_core.yaml` | `axs15231`, `sy6970`, `weather_helpers` | hardware drivers + helpers, always needed |
| `features/ble/ble_improv.yaml` | `nimble_base`, `nimble_improv`, `nimble_proxy`, `bluetooth_proxy` | only needed by the NimBLE variant |

Both point at `github://truffshuff/esphome-components@<commit>` pinned by full SHA. Keeping
the BLE components out of the core file means switching BLE stacks is a one-line change in
the package list.

`esphome config` reports: *"External components are overriding built-in components:
axs15231, sy6970"* — expected, that is the point of the fork.

---

## 7. BLE architecture

Three mutually exclusive packages; exactly one must be enabled because every one of them
defines `ble_scanner_switch`, which OTA and the weather fetch scripts turn off and on.

| Package | Stack | Notes |
|---|---|---|
| `ble_stub.yaml` | none | A no-op template switch. Smallest build. **Default in `Halo-v1.yaml`.** |
| `ble_improv.yaml` | NimBLE (fork) | Full proxy: `connection_slots`, scan duty tuning, ATT MTU 247. Allocates from PSRAM (`CONFIG_BT_NIMBLE_MEM_ALLOC_MODE_EXTERNAL`). |
| `ble_esphome.yaml` | Bluedroid (upstream) | ESPHome's native `esp32_ble_tracker` + `bluetooth_proxy` + `esp32_improv`. Tracks upstream fixes; uses more internal RAM. |

### WiFi/BT coexistence — do not remove

The ESP32-S3 has **one** 2.4 GHz radio shared between WiFi and BLE. Without software
arbitration the two collide and WiFi *reception* collapses while transmission still looks
perfectly healthy — an asymmetry that is very hard to read as a radio problem.

ESP-IDF sets `ESP_COEX_SW_COEXIST_ENABLE` to `default y` whenever WiFi and BT are both
enabled, so this used to be automatic. ESPHome 2026.7.0 (esphome#17008) replaced that with an
explicit reconciler that sets `CONFIG_SW_COEXIST_ENABLE` to **False** unless a component calls
`request_software_coexistence()` — and only `esp32_ble_tracker` does. A NimBLE build therefore
lost coexistence *silently* when this project moved from 2026.6.x to 2026.9.0.

What that looked like in practice, and why it took so long to find:

| Signal | Reading |
|---|---|
| RSSI / noise / AP satisfaction | −48 dBm / −96 dBm / 100 — all excellent |
| Packets (AP view) | 26,195 transmitted vs **330 received** |
| `rx_rate` | oscillating between the 6 Mbps basic-rate floor and 39 Mbps |
| OTA | ~0.6 kB/s, stalling at 18–24 %, or timing out during handshake |
| DNS | repeated `getaddrinfo` `EAI_FAIL` on inbound responses |
| Home Assistant API | mostly fine — small, retransmit-tolerant messages |

Every device-side diagnostic looked healthy: free heap ~102 KB, no component exceeding its
loop budget, `nimble_proxy send_failed=0`. **The fault was only visible from the AP**, in the
transmit/receive packet asymmetry. If inbound-only symptoms ever reappear, check the
controller's client stats before touching the firmware.

Fixed in two places, deliberately belt-and-braces:

- `nimble_base` calls `esp32.request_software_coexistence()` (fork commit `748cd7c`)
- `ble_improv.yaml` sets `CONFIG_SW_COEXIST_ENABLE: "y"` — `sdkconfig_options` wins over the
  reconciler's `set_idf_sdkconfig_default`, so it holds regardless of fork version

After the fix, the same OTA ran at ~22.7 kB/s (105 s for a 2.37 MB image) with `prepare` at
0.03 s. That is ~38× the broken figure, but still modest for WiFi — the transfer is lockstep
(8 KB block, then wait for a one-byte ACK), and `wifi: power_save_mode` is left at the ESP32
default of `light`, which adds DTIM-interval latency to every one of those ~290 round trips.
Setting `power_save_mode: none` is the obvious next experiment.

> `ble_esphome.yaml` sets `interval: 1100ms` / `window: 1100ms` — a 100 % scan duty cycle.
> ESPHome 2026.9.0 (esphome#18725) warns when the scan window exceeds 600 ms with WiFi
> enabled, and esphome#18356 documents missed advertisements in exactly this regime.
> If you switch to that package, reduce the window first.

---

## 8. Network architecture

| Layer | Configuration |
|---|---|
| WiFi | `networking.yaml` owns `wifi:`. `output_power: 15dB` (brownout headroom), `reboot_timeout: 0s`, AP fallback `"Halo Hotspot"` |
| `network:` | `enable_high_performance: false`, `tcp_send_buffer: 65535` — see MEMORY.md |
| API | Noise encryption, `reboot_timeout: 0s`, `max_connections: 8` |
| OTA | `ota: platform: esphome`, port 3232, password auth |
| Web server | port 80, version 2 (ESPHome default) |
| WireGuard | optional; full-tunnel capable, gated on a valid SNTP time |
| mDNS | enabled |

Two `reboot_timeout: 0s` settings are **load-bearing, not laziness**: the AQI offline history
ring buffer (`features/airq/airq_history.yaml`) lives in RAM and is wiped by a reboot. The
default WiFi (15 min) and API (15 min) reboot timeouts would destroy the buffer during exactly
the outages it exists to cover. A commented-out 30-minute API watchdog in `Halo-v1-Core.yaml`
is left in place as documentation of that decision.

### WireGuard re-handshake

The non-obvious part: calling `wireguard.enable()` on an already-enabled instance is a no-op
in libwg — it does not reset the crypto session. After a long WiFi outage the component is
still nominally "enabled" but stuck, and the server rejects its stale timestamps under
replay protection. `networking.yaml` therefore does an explicit `disable` → 500 ms →
`enable` on post-boot reconnect. `system_management.yaml` handles the other case: SNTP not
yet valid at boot, enable on first successful sync only (repeated re-initialisation churns
memory badly enough to trip the watchdog).

---

## 9. Validating local edits

Because packages come from GitHub, `esphome config Halo-v1.yaml` validates *the pushed
branch*, not your working tree. To validate local edits without pushing, generate a harness
that swaps `remote_packages:` for local `!include`s:

```bash
# in a scratch directory, symlink the real assets
ln -s <repo>/.../ESPHome/packages packages
ln -s <repo>/.../ESPHome/fonts fonts
ln -s <repo>/.../ESPHome/secrets.yaml secrets.yaml
ln -s <repo>/.../ESPHome/Halo-v1-Core.yaml Halo-v1-Core.yaml
```

then copy `Halo-v1.yaml` and replace its `packages:` block with:

```yaml
packages:
  p00_globals: !include packages/base/globals.yaml
  p01_esphome_core: !include packages/system/esphome_core.yaml
  # ... same order as the remote list ...
  p26_core: !include Halo-v1-Core.yaml
```

`esphome config <harness>.yaml` now resolves everything from disk. Remember this still does
not compile lambdas.

---

## 10. Known gaps

Recorded because they are easy to rediscover and misdiagnose.

1. **Weather page live values do not refresh on sensor change.**
   `weather_base.yaml` used to set a `sensor_ui_dirty` flag when the current-temperature,
   apparent-temperature, humidity, wind-speed and wind-direction sensors updated. The 2 s
   interval that consumed it only logged at DEBUG and cleared the flag — it never touched a
   widget. The dead machinery was removed in 2026.09. The observable behaviour is unchanged:
   the weather page repaints when `update_weather_display` runs (after a forecast fetch, and
   on page show), not when an individual sensor arrives. If live per-sensor updates are
   wanted, that is a **new feature**, not a regression.

2. **`Halo-v1-Core.yaml` breaks the modularity contract.**
   Core is a required file but references 44 ids that only optional feature packages define —
   page-rotation globals, WireGuard render flags, `computed_halo_aqi`, the wifi_status
   `*_needs_render` set. Commenting out `wireguard.yaml`, `page_rotation.yaml`,
   `wifi_status/*` or `airq_base.yaml` will fail validation with "Couldn't find ID". Only the
   BLE packages have a real stub. Fixing this properly means either stub packages for each
   feature or moving the consuming logic out of Core; both are larger refactors.
   `wifi_signal_db` was the one case that broke the **template's own default feature
   selection** — diagnostics is commented out in `Halo-v1.yaml`, so a new user hit
   "Couldn't find ID 'wifi_signal_db'" before changing anything. It was fixed by moving the
   sensor into `networking.yaml`. The working device file escaped it only because it enables
   diagnostics.

3. **The AppDaemon app targets a different unit on purpose.**
   `HomeAssistant/AppDaemon/apps/halo_sensor_history.yaml` sets
   `device_filter: "halo-v1-79e35c"` and the statistic ids in `halo_sensor_history.py` are
   `sensor.halo_v1_79e35c_*`, which is neither the device file in this repository
   (`halo-v1-79e384`) nor the unit currently being built. That is intentional, not a typo:
   the app exists for a Halo that lives at a remote site and is offline most of the time.
   When that unit comes back over WireGuard, `api: on_client_connected` in
   `Halo-v1-Core.yaml` runs `flush_aqi_history`, the device replays its in-RAM ring buffer as
   `esphome.aqi_history_batch` events, and this app backfills them into the HA recorder so
   the offline period is not a gap in the statistics.

   This is also *why* `api: reboot_timeout: 0s` and `wifi: reboot_timeout: 0s` are
   load-bearing (see §8) — a reboot during the outage would discard the very buffer this app
   exists to drain.

   Two things to know before adding a second offline unit: `device_filter` is configurable
   per app instance, but the `SENSORS` table in `halo_sensor_history.py` hard-codes the full
   `sensor.halo_v1_79e35c_*` statistic ids (lines 67+), so a second unit needs its own copy
   of that list — or the table refactored to take the entity prefix from `device_filter`.

4. **RESOLVED 2026-09-21 — the hourly forecast now arrives over the API, not HTTP.**
   This was a gap: `weather_base.yaml` subscribed to `sensor.hourly_forecast_esp`, which did
   not exist, so `fetch_hourly_forecast_http` was the only route and pulled the *entire*
   forecast — **220 entries, 63,204 bytes** — to keep 24, synchronously, blocking the main
   loop ~12 s per refresh.

   The reason the native action "always failed" was never a broken action: **ESPHome's API
   carries 16-bit frame lengths, so one message cannot exceed 65,535 bytes**
   (`APIBuffer::MAX_SIZE`). At 63.2 KB the payload sat ~2.3 KB under that ceiling — it would
   break the moment the provider extended its horizon.

   Fixed on the **Home Assistant side**, no firmware change:
   `config/templates/halo_hourly_forecast.yaml` is a trigger-based template sensor that calls
   `weather.get_forecasts` and publishes the first 24 entries on the `forecast` attribute of
   `sensor.hourly_forecast_esp`. Triggers: HA start, `/15`, and the event
   `halo_hourly_forecast_refresh` for on-demand refresh (a trigger-based template has no state
   until a trigger fires, so that event is how you populate it after a `template.reload`).

   Two things worth knowing if you touch it:

   - **`| to_json` is belt-and-braces, not load-bearing.** HA renders it, then parses the
     result back to a native list (`parse_result`), and `homeassistant/components/esphome`
     sends `str(attr_val)` — Python repr with *single* quotes. ArduinoJson's parser is lenient
     and accepts that; verified by compiling the exact 6,799-byte wire string against
     ArduinoJson 7.4.2. Keep `to_json` anyway: a future `None` or boolean field would render
     as `None`/`True`, which is neither valid JSON nor parseable.
   - **Renaming that sensor silently breaks the device.** It would simply never receive data.

   Verified on device: "Received hourly forecast attribute (6225 bytes)" → "Parsing 24 hourly
   entries from attribute" → stored, with no HTTP fallback triggered.

5. **All six Halo units share one VLAN, and none of them can resolve `ha_url` by name.**
   The controller lists `halo-v1-79d6b4` (.82 and .125), `79e1f8` (.104), `79e384` (.170),
   `79e294` (.242) and `79e35c` (.223) — every one on IoT VLAN 80, `10.140.80.0/24`, the same
   subnet as Home Assistant at `10.140.80.2`. Use HA's IP, not the reverse-proxy hostname
   (see §10.4 above and the comment in `halo-v1-79e384.yaml`). Two entries share the hostname
   `halo-v1-79d6b4` with different MACs, one of which has a non-Espressif OUI — probably a
   stale controller record worth cleaning up.

6. **`HomeAssistant/dashbaord.yaml`** is misspelled, and `printer_base.yaml` references it
   under that spelling. Renaming means updating the reference.

7. **`__pycache__/halo_sensor_history.cpython-314.pyc` is committed to git.** It should be
   removed from tracking and `__pycache__/` added to `.gitignore`.

8. **`*.bak` … `*.bak5` files exist under `packages/features/weather/`.** They are
   gitignored, so git cannot recover them, but `CLAUDE.md` forbids creating them. They are
   untracked local clutter.

---

## 11. Design decisions worth not undoing

| Decision | Reason |
|---|---|
| `api: reboot_timeout: 0s` and `wifi: reboot_timeout: 0s` | preserve the in-RAM AQI history buffer across outages |
| Backlight on plain GPIO, not LEDC | LEDC zero-duty crash on IDF 5.5 during boot |
| `touchscreen: update_interval: never` | interrupt-driven on GPIO11; polling is wasted work |
| `execute_from_psram: true` | lets display code run while flash is being written during OTA |
| `data_rate: 5MHz` on the display bus | conservative; raising it is a hardware-risk change, test carefully |
| SEN55 polled at 60 s | the sensor blocks the I²C bus for 4+ seconds per read |
| BLE scanner paused during OTA and weather fetches | avoids API send-buffer exhaustion during bursts |
| No `lv_refr_now()` in page-transition cleanup | a synchronous render from a script lambda corrupts the LVGL v9 pipeline and blacks out the screen |
