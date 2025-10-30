# Automatic Configuration with Jinja2 (ESPHome 2025.7.0+)

## 🚀 Revolutionary Feature: True Dynamic Configuration

Thanks to ESPHome 2025.7.0's Jinja2 support, your Halo configuration is now **100% automatic**. No more manual commenting/uncommenting of packages!

---

## How It Works

### The Magic: Jinja2 Ternary Operators

ESPHome now supports Jinja2 templating in substitutions, allowing conditional logic:

```yaml
substitutions:
  feature_ble: "false"

  # Automatically selects the right package!
  ble_package_file: '{{ "ble-core-simplified.yaml" if feature_ble | bool else "ble-stubs.yaml" }}'

packages:
  ble_auto:
    files:
      - packages/${ble_package_file}
```

**When `feature_ble = "false"`:** Loads `ble-stubs.yaml`
**When `feature_ble = "true"`:** Loads `ble-core-simplified.yaml`

---

## Configuration Examples

### Example 1: Enable BLE

**Change ONE line:**
```yaml
substitutions:
  feature_ble: "true"  # Changed from "false"
```

**Result:**
- ✅ `ble-core-simplified.yaml` automatically loaded
- ✅ All other features remain stubs
- ✅ Pages stay as clock-only
- ✅ BLE presence detection enabled!

### Example 2: Enable Weather (BLE Compatible!)

```yaml
substitutions:
  feature_weather: "true"
  feature_ble: "true"
```

**Result:**
- ✅ `weather-ha-actions.yaml` automatically loaded
- ✅ `ble-core-simplified.yaml` automatically loaded
- ✅ Pages automatically switch to `lvgl-pages-weather-forecast-only.yaml`
- ✅ BLE + Weather working together! (~108KB used, ~102KB free)

### Example 3: Full Featured (No BLE)

```yaml
substitutions:
  feature_airq: "true"
  feature_wifi_page: "true"
  feature_weather: "true"
  feature_ble: "false"  # Too much memory with all features
```

**Result:**
- ✅ `airq-display.yaml` loaded
- ✅ `wifi-display.yaml` loaded
- ✅ `weather-ha-actions.yaml` loaded
- ✅ `ble-stubs.yaml` loaded (disabled)
- ✅ Pages automatically: `lvgl-pages-airq-wifi-weather-forecast.yaml`
- ✅ All features except BLE! (~140KB used, ~70KB free)

---

## Automatic Package Selection

### Feature Packages

| Feature Flag | When TRUE | When FALSE |
|--------------|-----------|------------|
| `feature_airq` | `airq-display.yaml` | `airq-stubs.yaml` |
| `feature_ble` | `ble-core-simplified.yaml` | `ble-stubs.yaml` |
| `feature_wifi_page` | `wifi-display.yaml` | `wifi-stubs.yaml` |
| `feature_weather` | `weather-ha-actions.yaml` | `weather-stubs.yaml` |

### LVGL Pages (Automatic Detection!)

The pages package is selected based on your **combination** of features:

| Features Enabled | Auto-Selected Pages | BLE? |
|------------------|---------------------|------|
| None | `lvgl-pages-clock-only.yaml` | ✅ Yes |
| AirQ only | `lvgl-pages-airq-only.yaml` | ✅ Yes |
| WiFi only | `lvgl-pages-wifi-only.yaml` | ✅ Yes |
| WiFi + WireGuard | `lvgl-pages-wifi-wireguard.yaml` | ✅ Yes |
| AirQ + WiFi | `lvgl-pages-airq-wifi.yaml` | ✅ Yes |
| **Weather only** | `lvgl-pages-weather-forecast-only.yaml` | ✅ **YES!** |
| AirQ + Weather | `lvgl-pages-airq-weather-forecast-only.yaml` | ❌ No |
| WiFi + Weather | `lvgl-pages-wifi-weather-forecast-only.yaml` | ❌ No |
| AirQ + WiFi + Weather | `lvgl-pages-airq-wifi-weather-forecast.yaml` | ❌ No |

---

## The Jinja2 Pages Logic

Here's the smart logic that picks the right pages:

```yaml
lvgl_pages_file: >-
  {%- set airq = feature_airq | bool -%}
  {%- set wifi = feature_wifi_page | bool -%}
  {%- set wg = feature_wireguard | bool -%}
  {%- set weather = feature_weather | bool -%}

  {%- if not airq and not wifi and not weather -%}
    lvgl-pages-clock-only.yaml
  {%- elif airq and not wifi and not weather -%}
    lvgl-pages-airq-only.yaml
  {%- elif not airq and wifi and not wg and not weather -%}
    lvgl-pages-wifi-only.yaml
  {# ... more combinations ... #}
  {%- else -%}
    lvgl-pages-clock-only.yaml
  {%- endif -%}
```

This evaluates **at compile time** to select the perfect pages package!

---

## Complete Configuration Template

Here's your starting point:

```yaml
substitutions:
  # Device info
  name: "halo-v1-yourname"
  friendly_name: "Halo Display"

  # ========================================
  # FEATURE FLAGS - Just change these!
  # ========================================
  feature_clock: "true"              # Always enabled
  feature_airq: "false"              # Air quality
  feature_wifi_page: "false"         # WiFi info page
  feature_wireguard: "false"         # WireGuard VPN
  feature_weather: "false"           # Weather via HA actions
  feature_ble: "false"               # BLE presence

  # Weather config (if enabled)
  weather_entity_id: "weather.home"
  weather_refresh_interval: "300"

  # Network config
  wifi_ssid: !secret wifi_ssid
  wifi_password: !secret wifi_password
  api_encryption_key: "your-32-char-key-here"
  ota_password: "yourpassword"

  # ========================================
  # AUTOMATIC PACKAGE SELECTION (Don't touch!)
  # ========================================
  airq_package_file: '{{ "airq-display.yaml" if feature_airq | bool else "airq-stubs.yaml" }}'
  ble_package_file: '{{ "ble-core-simplified.yaml" if feature_ble | bool else "ble-stubs.yaml" }}'
  wifi_package_file: '{{ "wifi-display.yaml" if feature_wifi_page | bool else "wifi-stubs.yaml" }}'
  weather_package_file: '{{ "weather-ha-actions.yaml" if feature_weather | bool else "weather-stubs.yaml" }}'

  lvgl_pages_file: >-
    {%- set airq = feature_airq | bool -%}
    {%- set wifi = feature_wifi_page | bool -%}
    {%- set weather = feature_weather | bool -%}
    {%- if not airq and not wifi and not weather -%}
      lvgl-pages-clock-only.yaml
    {%- elif not airq and not wifi and weather -%}
      lvgl-pages-weather-forecast-only.yaml
    {%- elif airq and wifi and weather -%}
      lvgl-pages-airq-wifi-weather-forecast.yaml
    {%- else -%}
      lvgl-pages-clock-only.yaml
    {%- endif -%}

packages:
  halo_core:
    url: https://github.com/truffshuff/halo.git/
    files: [TFT_LCD/.../Halo-v1-Core.yaml]
    refresh: always

  # Auto-loading feature packages
  airq_auto:
    files: [packages/${airq_package_file}]
  ble_auto:
    files: [packages/${ble_package_file}]
  wifi_auto:
    files: [packages/${wifi_package_file}]
  weather_auto:
    files: [packages/${weather_package_file}]

  # Auto-loading pages
  lvgl_pages:
    files: [packages/${lvgl_pages_file}]
```

---

## Migration from Old Config

### Before (Manual):

```yaml
packages:
  # To enable BLE, you had to:
  # 1. Comment out ble_stubs
  # ble_stubs: !include ble-stubs.yaml

  # 2. Uncomment ble_core
  ble_core: !include ble-core-simplified.yaml
```

### After (Automatic):

```yaml
substitutions:
  feature_ble: "true"  # ONE LINE!

  # Package auto-selects
  ble_package_file: '{{ "ble-core-simplified.yaml" if feature_ble | bool else "ble-stubs.yaml" }}'

packages:
  ble_auto:
    files: [${ble_package_file}]  # Automatic!
```

---

## Benefits

### ✅ Simplicity
- Change ONE flag per feature
- No manual package management
- No commenting/uncommenting

### ✅ Correctness
- Can't forget to remove stubs when enabling features
- Can't forget to add stubs when disabling features
- Always consistent

### ✅ Intelligence
- Pages automatically match your feature set
- Memory-optimized configurations selected automatically
- BLE compatibility handled automatically

### ✅ Maintainability
- Easy to see what's enabled at a glance
- Less error-prone configuration
- Faster iteration when testing features

---

## Memory Budget Reference

| Configuration | RAM Used | Free RAM | BLE? |
|---------------|----------|----------|------|
| Clock only | ~45KB | ~165KB | ✅ Yes |
| Clock + BLE | ~105KB | ~105KB | ✅ Yes |
| Clock + Weather (HA Actions) | ~48KB | ~162KB | ✅ Yes |
| **Clock + BLE + Weather** | **~108KB** | **~102KB** | **✅ YES!** |
| Clock + AirQ + WiFi + Weather | ~140KB | ~70KB | ❌ No |

The automatic system ensures you never exceed memory limits by selecting appropriate packages!

---

## Troubleshooting

### Issue: Jinja2 not working

**Solution:** Ensure you're on ESPHome 2025.7.0 or later:
```bash
esphome version
```

Upgrade if needed:
```bash
pip install --upgrade esphome
```

### Issue: Wrong package selected

**Solution:** Check your feature flags are strings:
```yaml
feature_ble: "true"   # ✅ Correct (string)
feature_ble: true     # ❌ Wrong (boolean)
```

The `| bool` filter converts the string to boolean for the ternary operator.

### Issue: Pages don't match features

**Solution:** The pages logic checks for specific combinations. If you have an unusual combination, add it to the `lvgl_pages_file` logic or use a custom pages package.

---

## Advanced: Adding Custom Combinations

To add a new pages combination, edit the `lvgl_pages_file` substitution:

```yaml
lvgl_pages_file: >-
  {%- set airq = feature_airq | bool -%}
  {%- set wifi = feature_wifi_page | bool -%}
  {%- set weather = feature_weather | bool -%}

  {# Add your custom combination #}
  {%- if airq and wifi and not weather -%}
    lvgl-pages-custom-airq-wifi.yaml

  {# Existing combinations... #}
  {%- elif not airq and not wifi and not weather -%}
    lvgl-pages-clock-only.yaml
  {%- endif -%}
```

---

## Summary

With ESPHome 2025.7.0+ Jinja2 support, your Halo configuration is now:

🎯 **One-line feature toggling**
🤖 **Automatic package selection**
📦 **Intelligent pages matching**
💾 **Memory-optimized by default**
🔧 **Zero manual package management**

**Just change the feature flags and everything else happens automatically!** 🚀
