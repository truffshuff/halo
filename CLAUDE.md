# Halo v1 — Claude Guidelines

Guidelines for AI-assisted development on this repository.

---

## Project Overview

This is an **ESPHome YAML firmware project** for the LilyGo T-Display-Long (ESP32-S3 + 180×640 AMOLED). The entire firmware is expressed in YAML files — there is **no compiled application code** to run, build, or test locally. All validation happens by running `esphome compile` or `esphome run` against a physical device.

---

## Repository Structure

All firmware lives under:
```
halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/
```

Key files:
- `docs/ARCHITECTURE.md`, `docs/MEMORY.md` — read these first
- `Halo-v1.yaml` — **template**; the starting point users copy. Substitution values are placeholders. It must still validate as shipped.
- `Halo-v1-Core.yaml` — core LVGL display init and navigation glue
- `halo-v1-<mac>.yaml` — **a real working device config** (named for the unit's MAC suffix). `halo-v1-79e384.yaml` is the reference example and is authoritative over the template when they disagree.
- `packages/system/` — required system modules (do not disable)
- `packages/features/` — optional feature modules (comment/uncomment to toggle)
- `packages/base/globals.yaml` — must load before all other packages

The project uses ESPHome's `remote_packages` to pull configs from GitHub (`ref: modular`). Changes pushed to `modular` branch are picked up immediately by any device doing `refresh: Always`.

---

## Development Conventions

### YAML Style
- Every package file must have a structured header comment block documenting: purpose, features, dependencies, hardware requirements, memory usage, and creation date.
- Follow the pattern established in `packages/features/airq/airq_base.yaml` as a reference.
- Use `# ===...===` section dividers consistently.
- Comment all non-obvious configuration choices (see memory settings in `esphome_core.yaml`).

### Modular Architecture
- **System modules** (`packages/system/`) are required and provide no optional functionality.
- **Feature modules** (`packages/features/`) are fully optional; a user should be able to comment out any feature line in `Halo-v1.yaml` and have the firmware compile and run (with that feature absent).
- Each feature module must be self-contained: it declares its own globals, sensors, switches, scripts, and UI components.
- Feature modules must document their dependencies on other modules in their header.
- Load order matters: `globals.yaml` → system modules → feature modules → `page_rotation.yaml` (last).

### Secrets
- `secrets.yaml` is **never committed**. It is excluded in both `.gitignore` files.
- All sensitive values (WiFi credentials, HA tokens, WireGuard keys, API keys, OTA passwords) must use `!secret <key>` references.
- Never hardcode credentials, tokens, or private keys anywhere in committed files.
- If a secrets template is needed, use a `secrets.yaml.example` file with placeholder values.

### Backup Files
- Do **not** create `.bak` or `.bak[0-9]*` files. These patterns are excluded by `.gitignore`.
- Use git history (branches, commits, tags) for versioning and rollback instead.

---

## Common Tasks

### Adding a New Feature Module
1. Create `packages/features/<feature_name>/<feature_name>_base.yaml` for data/logic.
2. Create `packages/features/<feature_name>/<feature_name>_page.yaml` for the LVGL UI (if needed).
3. Add header documentation block to each file.
4. Add a page rotation global (`page_rotation_<Feature>_enabled`, `page_rotation_<Feature>_order`) if the feature adds a display page.
5. Update `Halo-v1.yaml` with a commented-out entry in the appropriate `packages:` section, with memory and hardware notes.
6. Update `README.md` memory reference table.

### Modifying the Display
- All LVGL UI is defined inside `esphome: on_boot` lambdas or `display:` lambdas within feature packages.
- The display is 180×640 pixels (portrait). LVGL objects use absolute coordinates.
- Pages are tracked via the `page_rotation` system; add new pages to that system's config.
- The display has a watchdog in `display_hardware.yaml` that forces a redraw if the display appears frozen.

### Memory Management
- **Read `TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/docs/MEMORY.md` before changing any buffer size, `sdkconfig_option`, or allocation.** Several values that look wasteful are load-bearing.
- Internal SRAM is the scarce resource (~320KB usable heap); PSRAM (8MB) is not. The metric that matters is *minimum free internal heap*, not idle free heap.
- WiFi and lwIP buffers must stay in **internal** SRAM (DMA). `CONFIG_SPIRAM_TRY_ALLOCATE_WIFI_LWIP` was tried and reverted — it made internal heap worse.
- LVGL draw buffers, the NimBLE stack, mbedTLS contexts, JSON pools and image buffers live in PSRAM.
- New JSON parsing must use the `ArduinoJson::Allocator` PSRAM pattern in `weather_base.yaml`, not `heap_caps_malloc(sizeof(JsonDocument), ...)` + placement new (that only moves ~32 bytes).
- Never allocate inside a 1s or 100ms interval lambda. Use `static char buf[N]` + `snprintf`, or fixed stack arrays.
- Before adding a new feature, estimate its memory impact and document it in the package header — and say whether it is flash, internal SRAM or PSRAM.
- The watchdog timeout is 30 seconds (extended for large JSON parsing in weather). Do not introduce blocking operations longer than this.
- Disabling features like `weather_hourly.yaml` saves ~7KB of flash — mention this in feature comments.

### WireGuard
- The WireGuard module (`features/wireguard/wireguard.yaml`) contains server-specific configuration substitutions.
- Users must edit their local `Halo-v1.yaml` substitutions (via `secrets.yaml`) before enabling this module.
- Never commit WireGuard private keys or peer endpoints.

---

## What Not to Do

- **Do not run `esphome run`, `esphome upload`, or `esphome logs`** — these touch a physical device. `esphome config` (read-only validation) is fine when an ESPHome install and `secrets.yaml` are present; prefer a pinned version matching `min_version`. Note that `esphome config` does **not** compile lambdas, so it cannot validate embedded C++.
- **Do not assume local edits under `packages/` are what gets built** — builds pull from GitHub `ref: modular` with `refresh: Always`. Use the local-path harness in `docs/ARCHITECTURE.md` §9 to validate uncommitted changes.
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
- Tag stable releases: `git tag v26.02.20` before pushing to allow pinning.

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
| WiFi TX power | 15dBm |
| LVGL buffer | 50% of screen → 1/2-screen buffer in PSRAM (see docs/MEMORY.md §4) |
| TCP send buffer | 65535 via `network: tcp_send_buffer:` |
| Home Assistant API reboot timeout | 0s (disabled — preserves the in-RAM AQI history buffer) |
| WiFi reboot timeout | 0s (same reason) |
