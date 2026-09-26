# Halo v1 — Claude Guidelines

Guidelines for AI-assisted development on this repository.

---

## Project Overview

This is an **ESPHome YAML firmware project** for the LilyGo T-Display-Long (ESP32-S3 + 180×640 AMOLED). The entire firmware is expressed in YAML files plus C++ lambdas embedded in them — there is **no compiled application code** to run, build, or test locally. All validation happens by running `esphome compile` or `esphome run` against a physical device.

The only external C++ is the companion repo **truffshuff/esphome-components** (local clone: `../../esphome-components`, i.e. `~/Documents/GITHUB/esphome-components`; working branch `nimble-bluedroid-parity`). It has its own `CLAUDE.md`.

---

## Repository Structure

All firmware lives under:
```
halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/
```

Key files:
- `docs/ARCHITECTURE.md`, `docs/MEMORY.md` — read these first. ARCHITECTURE §10 "Known gaps" is the running list of known defects and doc-vs-code mismatches; add to it rather than leaving findings only in chat.
- `Halo-v1.yaml` — **template**; the starting point users copy. Substitution values are placeholders. It must still validate as shipped.
- `Halo-v1-Core.yaml` — the `lvgl:` block, page navigation (1 s rotation + 100 ms page-show chain), the `api:` block and its triggers. Loaded **last**.
- `halo-v1-<mac>.yaml` — **a real working device config** (named for the unit's MAC suffix). `halo-v1-79e384.yaml` is the reference example and is authoritative over the template when they disagree.
- `secrets.yaml.example` — every secret key the template needs, with placeholders.
- `packages/system/` — required system modules
- `packages/features/` — feature modules
- `packages/base/globals.yaml` — loaded first
- `HomeAssistant/AppDaemon/apps/` — AppDaemon app that consumes the AQI history events (targets unit `79e35c` on purpose; ARCHITECTURE §10 gap 3)

The project uses ESPHome's `remote_packages` to pull configs from GitHub (`ref: modular`). Changes pushed to `modular` branch are picked up immediately by any device doing `refresh: Always`.

---

## Development Conventions

### YAML Style
- Every package file must have a structured header comment block documenting: purpose, features, dependencies, hardware requirements, memory usage (say flash / internal SRAM / PSRAM), and creation date.
- Follow the pattern established in `packages/features/airq/airq_base.yaml` as a reference.
- Use `# ===...===` section dividers consistently.
- Comment all non-obvious configuration choices (see memory settings in `esphome_core.yaml`).
- **Dependencies in headers must name the file that actually defines each id.** Past headers pointed at `base/globals.yaml`, "Core" or files that no longer exist (`base/sensors.yaml`, `packages/pages/`). Check with `git grep -n "id: <name>"` before writing one.

### Modular Architecture — the goal vs. today
- **Goal:** a user can comment out any feature line in `Halo-v1.yaml` and still compile.
- **Today:** only `diagnostics.yaml` and the choice of BLE package can be toggled. Core's rotation list and page-show chain name every page and its `page_rotation_*` globals, and several required modules reach into optional ones (networking → wireguard, weather_led_effects; system_management → wireguard; weather_base → weather_page/daily/hourly scripts). The full table is ARCHITECTURE §10 gap 2. Don't tell users a package is optional without checking that table; don't make the coupling worse.
- Each feature module should be self-contained: it declares its own globals, sensors, switches, scripts, and UI components, and documents its dependencies in its header.
- Load order: `globals.yaml` → system modules → feature modules → `Halo-v1-Core.yaml` (last). LVGL pages appear, and `lvgl.page.next` cycles, in package merge order.
- Some ids live in surprising places; check before assuming: the "Display Backlight" light is in `weather_led_effects.yaml`; AirQ's rotation switch/number are in `page_rotation.yaml`; the Auto Page Rotation master switch, reboot buttons and display watchdog are in `diagnostics.yaml`.

### Secrets
- `secrets.yaml` is **never committed**. It is excluded in both `.gitignore` files.
- All sensitive values (WiFi credentials, HA tokens, WireGuard keys, API keys, OTA passwords) must use `!secret <key>` references.
- Never hardcode credentials, tokens, or private keys anywhere in committed files.
- Keep `secrets.yaml.example` in sync when adding or renaming a `!secret` key. Read key *names* from a real `secrets.yaml` with `grep -oE "^[A-Za-z0-9_]+:"` — never print its values.

### Backup Files
- Do **not** create `.bak` or `.bak[0-9]*` files. These patterns are excluded by `.gitignore`.
- Use git history (branches, commits, tags) for versioning and rollback instead. The same goes for large commented-out "kept for rollback" code blocks: delete them and cite the commit that still has them.

---

## Common Tasks

### Adding a New Feature Module
1. Create `packages/features/<feature_name>/<feature_name>_base.yaml` for data/logic.
2. Create `packages/features/<feature_name>/<feature_name>_page.yaml` for the LVGL UI (if needed).
3. Add header documentation block to each file.
4. If it adds a display page: add `page_rotation_<Feature>_enabled` / `_order` globals, a switch and order number, **and** wire it into `Halo-v1-Core.yaml` — a `push(...)` line in the 1 s rotation lambda (raise `MAX_PAGES` if needed) and a branch in the 100 ms page-show chain. Ideally give it a stub so the page stays optional.
5. Update `Halo-v1.yaml` (and the reference device file) with an entry in the appropriate `packages:` section, with memory and hardware notes.
6. Update `README.md` memory reference table.

### Modifying the Display
- LVGL UI is declared under `lvgl: pages:` in each feature's `*_page.yaml`; updates happen through `lvgl.label.update` / `lvgl.widget.update` actions in scripts, intervals and `on_value` triggers.
- The display is 180×640 pixels (portrait). LVGL objects use absolute coordinates.
- Follow the `*_last_text` / `*_needs_render` pattern: only repaint when the text changed and the page is showing.
- The display watchdog is in `features/diagnostics/diagnostics.yaml` (not `display_hardware.yaml`). Its heartbeat is the clock's `time_update`, not a real flush callback.
- Don't call `lv_refr_now()` from script lambdas (LVGL v9 corruption; ARCHITECTURE §11).

### Memory Management
- **Read `TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/docs/MEMORY.md` before changing any buffer size, `sdkconfig_option`, or allocation.** Several values that look wasteful are load-bearing.
- Internal SRAM is the scarce resource (~320KB usable heap); PSRAM (8MB) is not. The metric that matters is *minimum free internal heap*, not idle free heap.
- WiFi and lwIP buffers must stay in **internal** SRAM (DMA). `CONFIG_SPIRAM_TRY_ALLOCATE_WIFI_LWIP` was tried and reverted — it made internal heap worse.
- LVGL draw buffers, the NimBLE stack, mbedTLS contexts, JSON pools, image buffers and the AQI history ring buffer live in PSRAM.
- New JSON parsing must use the `ArduinoJson::Allocator` PSRAM pattern in `weather_base.yaml`, not `heap_caps_malloc(sizeof(JsonDocument), ...)` + placement new (that only moves ~32 bytes).
- Never allocate inside a 1s or 100ms interval lambda. Use `static char buf[N]` + `snprintf`, or fixed stack arrays.
- Before adding a new feature, estimate its memory impact and document it in the package header — and say whether it is flash, internal SRAM or PSRAM. Compute sizes from the code (e.g. struct size × count), don't copy old estimates: the AQI history buffer was documented as 2.3 KB for months while the code allocated ~270 KB.
- The watchdog timeout is 30 seconds (extended for large JSON parsing in weather). `http_request` is synchronous. Do not introduce blocking operations longer than this.
- One ESPHome API message cannot exceed 65,535 bytes. Size anything sent or received over the API (events, attributes, action responses) below that.

### Home Assistant side
The firmware depends on HA configuration that is **not in this repo**:
- "Allow the device to perform Home Assistant actions" on the ESPHome device (daily forecast via `homeassistant.action`).
- A trigger-based template sensor `sensor.hourly_forecast_esp` whose `forecast` attribute holds 24 hourly entries. The README has an equivalent template; the running one is the user's `config/templates/halo_hourly_forecast.yaml`.
- Optionally the AppDaemon app for AQI history backfill.

### WireGuard
- The WireGuard module (`features/wireguard/wireguard.yaml`) reads its server configuration from `wg_*` secrets (see `secrets.yaml.example`).
- The package cannot currently be commented out; users without a VPN keep it and leave the "Enable WireGuard" switch off (placeholder secrets are fine).
- Never commit WireGuard private keys or peer endpoints.

### External components (truffshuff/esphome-components)
- Pinned by full SHA in **two** places that must match: `packages/system/esphome_core.yaml` (`axs15231`, `weather_helpers`) and `packages/features/ble/ble_improv.yaml` (NimBLE stack + vendored proxy). A fork change does nothing until both pins move and `modular` is pushed.
- `ble_device_base`, `bluetooth_connection` and `bluetooth_proxy` in the fork are vendored ESPHome 2026.9.0 copies that override the built-ins; only `ble_improv.yaml` may list them, never alongside `ble_esphome.yaml`.

---

## Validation

- **Do not run `esphome run`, `esphome upload`, or `esphome logs`** — these touch a physical device. `esphome config` (read-only validation) is fine when an ESPHome install ≥ `min_version` (2026.9.0) and `secrets.yaml` are present; an older install fails the version check before validating anything. `esphome config` does **not** compile lambdas, so it cannot validate embedded C++.
- **Do not assume local edits under `packages/` are what gets built** — builds pull from GitHub `ref: modular` with `refresh: Always`. Use the local-path harness in `docs/ARCHITECTURE.md` §9 to validate uncommitted changes.
- For any lambda change, `esphome compile` on the harness (ARCHITECTURE §9 has a generator script) is the real check — it builds only and touches no device. Compile `halo-v1-79e384.yaml` (NimBLE + diagnostics, the widest build) and `esphome config` the template too.
- For comment/doc-only YAML changes, a cheap proof when `esphome config` isn't available: parse every file with PyYAML (a SafeLoader with `add_multi_constructor('!', ...)` to accept `!secret`/`!lambda`) and compare the parsed structure against `git show HEAD:<file>`. Identical structure = only comments changed. Lambda-internal `//` comments show as a difference; inspect those lines with `git diff -U0`.

---

## What Not to Do

- **Do not create backup files** — use git instead.
- **Do not hardcode local network addresses** (e.g., `192.168.x.x`) or device-specific sensor entity IDs in shared package files. Those belong in substitutions within `Halo-v1.yaml` or device-specific overrides.
- **Do not commit `secrets.yaml`** under any circumstances.
- **Do not remove or alter the `.gitignore` entries** for `secrets.yaml` and `.bak` files.
- **Do not duplicate external_components declarations** across packages. They are consolidated in `esphome_core.yaml` for hardware drivers. BLE-specific components are declared inside their respective BLE package.

---

## Git Workflow

- Active branch: `modular`
- Upstream: `yashmulgaonkar/halo` (remote: `upstream`)
- Fork: `truffshuff/halo` (remote: `origin`)
- Changes pushed to `modular` are immediately live for devices using `ref: modular` and `refresh: Always`.
- Use descriptive commit messages (imperative mood, present tense): `Add WireGuard page display`, `Fix BLE scanner state on OTA`.
- Tag stable releases with the date: `git tag vYY.MM.DD` (e.g. `v26.09.21`) before pushing to allow pinning.

---

## Key Constraints

| Constraint | Value |
|-----------|-------|
| ESPHome minimum version | 2026.9.0 |
| Build toolchain | native `esp-idf` (default since 2026.7.0; `platformio` is deprecated, removed 2027.2.0) |
| Display resolution | 180 × 640 px |
| ESP32-S3 flash | 16MB |
| ESP32-S3 PSRAM | 8MB (octal, 80MHz) |
| CPU frequency | 240MHz |
| Watchdog timeout | 30 seconds |
| WiFi TX power | 15dBm (measured; see the `wifi:` comment in the entry files) |
| LVGL buffer | 50% of screen → 1/2-screen buffer in PSRAM (see docs/MEMORY.md §4) |
| TCP send buffer | 65535 via `network: tcp_send_buffer:` |
| API message size | ≤ 65,535 bytes |
| Home Assistant API reboot timeout | 0s (disabled — preserves the in-RAM AQI history buffer) |
| WiFi reboot timeout | 0s (same reason) |
