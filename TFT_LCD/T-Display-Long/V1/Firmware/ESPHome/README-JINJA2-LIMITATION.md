# Jinja2 Dynamic Configuration - Important Limitation

## TL;DR

**The revolutionary Jinja2 automatic package selection ONLY works with ESPHome CLI, not Home Assistant dashboard.**

## The Problem

Home Assistant's ESPHome dashboard processes YAML in this order:

1. Parse `!include` statements → **ERROR if file path contains `${...}`**
2. Evaluate Jinja2 substitutions → **Too late!**

This means file paths like:
```yaml
packages:
  airq_auto: !include packages/${airq_package_file}
```

Result in error:
```
Error reading file /config/esphome/packages/${airq_package_file}
```

## Why ESPHome CLI Works

ESPHome CLI processes YAML correctly:

1. Evaluate Jinja2 substitutions → `${airq_package_file}` becomes `airq-stubs.yaml`
2. Parse `!include` statements → `!include packages/airq-stubs.yaml` ✅

## Solutions

### Option 1: Use ESPHome CLI (RECOMMENDED)

**File:** `halo-v1-79e384.yaml`

**Features:**
- ✅ Full Jinja2 dynamic configuration
- ✅ Change one flag, entire config updates
- ✅ Impossible to mismatch packages and pages
- ✅ Fast compilation
- ✅ Supports multiple devices easily

**Installation:**
```bash
pip install esphome

# Or with Home Assistant venv
source /srv/homeassistant/bin/activate
pip install esphome
```

**Usage:**
```bash
cd /path/to/halo/Firmware/ESPHome
esphome compile halo-v1-79e384.yaml
esphome upload halo-v1-79e384.yaml
```

---

### Option 2: Use HA Dashboard with Manual Config

**File:** `halo-v1-79e384-ha-manual.yaml`

**Features:**
- ✅ Web-based interface
- ✅ Integrated with Home Assistant
- ❌ Manual package comment/uncomment required
- ❌ Easy to forget to update pages package
- ❌ More error-prone

**Example:**
```yaml
packages:
  # === Air Quality Monitoring ===
  # Uncomment ONE:
  # airq_enabled: !include packages/airq-display.yaml      # Enable AirQ
  airq_disabled: !include packages/airq-stubs.yaml         # Disable AirQ

  # === Weather Display ===
  # Uncomment ONE:
  # weather_enabled: !include packages/weather-ha-actions.yaml  # Enable
  weather_disabled: !include packages/weather-stubs.yaml         # Disable

  # === LVGL Pages (MUST MATCH!) ===
  # Choose ONE based on enabled features above:
  lvgl_pages: !include packages/lvgl-pages-clock-only.yaml
  # lvgl_pages: !include packages/lvgl-pages-weather-forecast-only.yaml  # If weather enabled
```

**Steps to enable a feature:**
1. Comment out the `_disabled` line
2. Uncomment the `_enabled` line
3. Update the `lvgl_pages` line to match
4. Compile

**Steps to disable a feature:**
1. Comment out the `_enabled` line
2. Uncomment the `_disabled` line
3. Update the `lvgl_pages` line to match
4. Compile

---

## Comparison

| Aspect | ESPHome CLI | HA Dashboard Manual |
|--------|-------------|---------------------|
| **Dynamic Config** | ✅ Automatic | ❌ Manual editing |
| **Feature Flags** | ✅ Just change true/false | ❌ Must comment/uncomment |
| **Error Prone** | ✅ Low | ❌ High (easy to mismatch) |
| **Multi-Device** | ✅ Easy | ❌ Tedious |
| **Setup** | ✅ One-time pip install | ❌ Copy files to HA |
| **Interface** | Terminal | Web UI |
| **Speed** | ✅ Fast | Medium |

## Examples

### Enabling Weather - CLI Method

**File:** `halo-v1-79e384.yaml`

```yaml
substitutions:
  feature_weather: "true"  # ← Change this one line
  weather_entity_id: "weather.home"
```

**Result:**
- Automatically loads `weather-ha-actions.yaml` ✅
- Automatically loads correct pages package ✅
- Automatically updates default page options ✅

**One line changed, everything updates!**

---

### Enabling Weather - HA Dashboard Method

**File:** `halo-v1-79e384-ha-manual.yaml`

**Step 1:** Uncomment weather package
```yaml
  # weather_disabled: !include packages/weather-stubs.yaml  # ← Comment this
  weather_enabled: !include packages/weather-ha-actions.yaml  # ← Uncomment this
```

**Step 2:** Update pages package
```yaml
  # lvgl_pages: !include packages/lvgl-pages-clock-only.yaml  # ← Comment old
  lvgl_pages: !include packages/lvgl-pages-weather-forecast-only.yaml  # ← Uncomment new
```

**Step 3:** Update default page options
```yaml
select:
  - platform: template
    name: "Default Page at Boot"
    options:
      - "Vertical Clock"
      - "Weather"  # ← Add this
```

**Step 4:** Update substitution
```yaml
substitutions:
  weather_entity_id: "weather.home"  # ← Add/update this
```

**Four places to edit, easy to forget one!**

---

## Recommendation

**Use ESPHome CLI!** The Jinja2 automatic configuration is:

1. **Safer** - Impossible to mismatch packages
2. **Faster** - Change one line vs four
3. **Clearer** - Feature flags are self-documenting
4. **Better for multiple devices** - Copy template, change flags, done!

## Installation Guide

### Linux/macOS:
```bash
pip3 install esphome
```

### Windows:
```powershell
pip install esphome
```

### With Home Assistant venv:
```bash
source /srv/homeassistant/bin/activate
pip install esphome
```

### Docker:
```bash
docker pull esphome/esphome
docker run --rm -v "${PWD}":/config -it esphome/esphome compile halo-v1-79e384.yaml
```

## Files Reference

| File | Purpose | Method |
|------|---------|--------|
| `halo-v1-79e384.yaml` | Jinja2 automatic config | ESPHome CLI only |
| `halo-v1-79e384-ha-manual.yaml` | Manual package selection | HA Dashboard |
| `halo-v1-template-dynamic.yaml` | Template for new devices | ESPHome CLI only |

## Bottom Line

The Jinja2 dynamic configuration is **revolutionary** for managing Halo devices with different feature sets.

**Don't let HA dashboard limitations prevent you from using it!** Install ESPHome CLI and enjoy the automatic configuration experience.

**ESPHome CLI setup takes 2 minutes. Managing manual packages takes 2 minutes per change.**

Choose wisely! 🚀
