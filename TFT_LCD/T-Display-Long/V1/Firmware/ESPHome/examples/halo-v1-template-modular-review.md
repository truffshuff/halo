# halo-v1-template-modular.yaml — Quick Review & Actionable Recommendations

Summary
- Keep the template lean: move large commented blocks and rarely-used example packages into a dedicated "examples" or "extras" folder.
- Remove or fix any placeholder removal directives and broken script entries.
- Improve documentation clarity around memory, BLE compatibility, and custom page creation.

Files / script entries to remove or relocate
- Move out of the main template (place in /docs/examples or /packages-extras):
  - All large commented package blocks used only as examples (wireguard*, ble_*, weather_stubs, wifi_stubs, and commented weather sensor blocks). They clutter the primary template and are easy to re-add from examples.
  - Duplicate or alternative lvgl_pages blocks (keep only the active/default lvgl_pages block and reference other options in examples).
- Remove or replace broken/placeholder script entry:
  - script: - id: !remove time_update
    - This is not a valid script definition. Remove it or replace with a proper script stub or a comment instructing how to remove the time_update script.
- Remove stale commented remote references if not maintained:
  - If dashboard_import and remote_package always point to the same repo, consider keeping only the minimal reference and moving the rest to examples.
- Remove unused stub packages from the main packages list (if not used by your chosen config):
  - e.g., if BLE will never be used in your deployments, remove ble_coexist_tuning / ble_core_simplified references from the template and document how to add them back.

Improvements to configuration & safety checks
- Validate dependencies programmatically or document checks:
  - Add a short validation checklist or a tiny script (CI) that ensures required dependencies are enabled:
    - psram_helpers present before any weather packages
    - weather_sensors_daily and weather_sensors_hourly require weather_sensors_current
    - lvgl_pages must match the selected weather/sensor packages
- Replace in-file commented choices with a clear "CONFIGURATION CHECKLIST" at top:
  - Single-line checkboxes (or YAML anchors) that say which section to enable/disable for a given config number.
- Use explicit "enabled: true/false" flags for optional feature groups (wifi, airq, weather, ble, wireguard) to make automated validation simpler.
- Normalize naming & placeholders:
  - Make the device name/friendly_name instructions more prominent with a single, unmistakable TODO line.
  - Suggest using name_add_mac_suffix: true for unique network names when deploying many units.

Documentation improvements (clarity & brevity)
- Create a one-page "Quick Start" at top of repo:
  - Minimal steps to get a working firmware for most users (Config 1 or 7), including memory-safe defaults.
- Memory and BLE guidance:
  - Condense the long config table into a short "If you need BLE -> use configs 1-7 only" plus recommended buffer_size per scenario. Example:
    - BLE: buffer_size: 15%
    - Full weather (no BLE): buffer_size: 25%
    - Clock-only: buffer_size: 35%
- Custom lvgl_pages guidance:
  - Add step-by-step instructions to create a custom lvgl_pages package:
    1. Copy existing pages file.
    2. Remove unwanted page includes.
    3. Update page rotation logic and select options.
    4. Run esphome compile to check size.
- Add a "Troubleshooting / Common Errors" section:
  - Out-of-memory on compile — check which large sensor packages are enabled.
  - BLE failing to start — ensure buffer_size is 15% and remove large weather sensor sets.
  - Page mismatch — ensure default select options match enabled lvgl pages.
- Standardize capitalisation and terms: use "WireGuard", "BLE", "PSRAM" uniformly.
- Provide commands to test/compile:
  - Example esphome compile command and recommended PlatformIO upload_speed check.

Minor YAML fixes and style
- Remove or reformat long inline comments that duplicate content — move long explanations into a README.
- Replace commented blocks that users might accidentally uncomment with examples in an examples folder (less risk of accidental enabling).
- Consider adding a small comment near each large package indicating the approximate flash/ram cost (already partly present; make it consistent).
- Consider adding a `check: true` or `validate:` section (for repo-level CI or a small Python/Node script) that uses simple heuristics to warn when a selected combination will likely blow past memory.

Suggested next steps
- Create /docs/examples/ with:
  - wireguard-examples.md (3-package enable instructions)
  - ble-examples.md (BLE safe configs and memory tips)
  - custom-pages-tutorial.md (how to create custom lvgl_pages)
- Replace the broken script entry and run a test compile for your most common configs (1, 7, 12).
- Add a small CI job that runs esphome compile for at least one BLE and one Full-Weather configuration.

If you want, I can:
- Generate a cleaned template with only the active/default packages and an examples directory.
- Produce the short "Quick Start" and "Custom Pages" docs for /docs/examples.
