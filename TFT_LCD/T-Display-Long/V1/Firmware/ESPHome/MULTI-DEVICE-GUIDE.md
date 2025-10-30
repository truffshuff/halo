# Multi-Device Configuration Guide

## Overview

The Jinja2 dynamic configuration system is **perfect for managing multiple Halo devices** with different feature sets. Each device gets its own YAML file with its own feature flags.

## Architecture

```
Firmware/ESPHome/
├── Halo-v1-Core.yaml           # Shared core (hardware, components)
├── packages/                    # Shared packages (all features)
│   ├── airq-display.yaml
│   ├── ble-core-simplified.yaml
│   ├── weather-ha-actions.yaml
│   └── (all other packages)
│
├── halo-v1-79e384.yaml         # Device 1: Office (Clock + AirQ + WiFi)
├── halo-v1-abc123.yaml         # Device 2: Bedroom (Clock + Weather + BLE)
├── halo-v1-def456.yaml         # Device 3: Kitchen (Clock only)
└── halo-v1-ghi789.yaml         # Device 4: Living Room (Full features)
```

## How It Works

### 1. Each Device Has Its Own File

Each physical Halo device gets its own YAML file with a unique:
- **Device name** (e.g., `halo-v1-79e384`)
- **MAC address** (in the filename)
- **Feature configuration** (different per device!)

### 2. All Devices Share:

✅ **Core hardware config** (`Halo-v1-Core.yaml`) - ESP32, display, touchscreen, etc.
✅ **Package files** (`packages/` directory) - All features available to all devices
✅ **Jinja2 logic** - Same automatic package selection mechanism

### 3. Each Device Chooses Features Independently

Device files only differ in their **substitutions** section:

**Example: Office Device (halo-v1-79e384.yaml)**
```yaml
substitutions:
  name: "halo-office"
  friendly_name: "Office Air Quality Monitor"

  # Office needs AirQ monitoring
  feature_airq: "true"          # ✅ Enabled
  feature_wifi_page: "true"     # ✅ Enabled
  feature_weather: "false"      # ❌ Not needed
  feature_ble: "false"          # ❌ No BLE devices here

  wifi_ssid: !secret office_wifi_ssid
  wifi_password: !secret office_wifi_password
  api_encryption_key: "office_key_here"
```

**Example: Bedroom Device (halo-v1-abc123.yaml)**
```yaml
substitutions:
  name: "halo-bedroom"
  friendly_name: "Bedroom Weather Display"

  # Bedroom needs weather and BLE tracking
  feature_airq: "false"         # ❌ Don't need AirQ
  feature_wifi_page: "false"    # ❌ Don't need WiFi page
  feature_weather: "true"       # ✅ Show weather!
  feature_ble: "true"           # ✅ Track sleep tracker

  weather_entity_id: "weather.home"
  wifi_ssid: !secret home_wifi_ssid
  wifi_password: !secret home_wifi_password
  api_encryption_key: "bedroom_key_here"
```

**Example: Kitchen Device (halo-v1-def456.yaml)**
```yaml
substitutions:
  name: "halo-kitchen"
  friendly_name: "Kitchen Clock"

  # Kitchen just needs a clock
  feature_airq: "false"         # ❌ Minimal features
  feature_wifi_page: "false"    # ❌ Just a clock
  feature_weather: "false"      # ❌ Keep it simple
  feature_ble: "false"          # ❌ No tracking

  wifi_ssid: !secret home_wifi_ssid
  wifi_password: !secret home_wifi_password
  api_encryption_key: "kitchen_key_here"
```

## Compilation Methods for Multiple Devices

### Method 1: ESPHome CLI (Local) - BEST FOR MULTIPLE DEVICES

**Setup once:**
```bash
cd /path/to/halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome
```

**Compile each device:**
```bash
# Office device
esphome compile halo-v1-79e384.yaml
esphome upload halo-v1-79e384.yaml --device 192.168.1.100

# Bedroom device
esphome compile halo-v1-abc123.yaml
esphome upload halo-v1-abc123.yaml --device 192.168.1.101

# Kitchen device
esphome compile halo-v1-def456.yaml
esphome upload halo-v1-def456.yaml --device 192.168.1.102
```

**Advantages:**
- ✅ All device files in one directory
- ✅ All share the same core and packages
- ✅ Easy to compare configurations
- ✅ Fast compilation
- ✅ Can script batch updates

---

### Method 2: Home Assistant ESPHome Dashboard

**Setup:**

Your Home Assistant `/config/esphome/` directory contains:

```
/config/esphome/
├── Halo-v1-Core.yaml            # Shared by all devices
├── packages/                     # Shared by all devices
│   └── (all package files)
│
├── halo-v1-79e384.yaml          # Office device config
├── halo-v1-abc123.yaml          # Bedroom device config
├── halo-v1-def456.yaml          # Kitchen device config
└── halo-v1-ghi789.yaml          # Living room device config
```

**Workflow:**

1. Copy all files to `/config/esphome/` (one time)
2. All devices appear in ESPHome dashboard
3. Compile each device independently from the dashboard
4. Each device loads **different packages** based on its feature flags!

**Advantages:**
- ✅ Web interface for all devices
- ✅ Integrated with Home Assistant
- ✅ Easy OTA updates
- ✅ Visual compilation status

---

## Example: 4-Device Home Setup

### Device Configuration Matrix

| Device | Location | Features | RAM Usage | BLE Compat? |
|--------|----------|----------|-----------|-------------|
| `halo-office` | Office | AirQ + WiFi | ~125KB | ❌ No |
| `halo-bedroom` | Bedroom | Weather + BLE | ~108KB | ✅ Yes |
| `halo-kitchen` | Kitchen | Clock only | ~45KB | ✅ Yes |
| `halo-living` | Living Room | All features | ~140KB | ❌ No |

### File Contents

**halo-v1-office.yaml (79e384)**
```yaml
substitutions:
  name: "halo-office"
  friendly_name: "Office Monitor"
  feature_airq: "true"
  feature_wifi_page: "true"
  feature_weather: "false"
  feature_ble: "false"
  api_encryption_key: "office_api_key_here"

packages:
  halo_v1_core: !include Halo-v1-Core.yaml
  airq_auto: !include packages/${airq_package_file}      # Loads airq-display.yaml
  ble_auto: !include packages/${ble_package_file}        # Loads ble-stubs.yaml
  wifi_auto: !include packages/${wifi_package_file}      # Loads wifi-display.yaml
  weather_auto: !include packages/${weather_package_file} # Loads weather-stubs.yaml
  lvgl_pages: !include packages/${lvgl_pages_file}       # Loads lvgl-pages-airq-wifi.yaml

wifi:
  ssid: !secret office_wifi_ssid
  password: !secret office_wifi_password

# ... rest of device config
```

**halo-v1-bedroom.yaml (abc123)**
```yaml
substitutions:
  name: "halo-bedroom"
  friendly_name: "Bedroom Display"
  feature_airq: "false"
  feature_wifi_page: "false"
  feature_weather: "true"
  feature_ble: "true"
  weather_entity_id: "weather.home"
  api_encryption_key: "bedroom_api_key_here"

packages:
  halo_v1_core: !include Halo-v1-Core.yaml
  airq_auto: !include packages/${airq_package_file}      # Loads airq-stubs.yaml
  ble_auto: !include packages/${ble_package_file}        # Loads ble-core-simplified.yaml
  wifi_auto: !include packages/${wifi_package_file}      # Loads wifi-stubs.yaml
  weather_auto: !include packages/${weather_package_file} # Loads weather-ha-actions.yaml
  lvgl_pages: !include packages/${lvgl_pages_file}       # Loads lvgl-pages-weather-forecast-only.yaml

wifi:
  ssid: !secret home_wifi_ssid
  password: !secret home_wifi_password

# ... rest of device config
```

**halo-v1-kitchen.yaml (def456)**
```yaml
substitutions:
  name: "halo-kitchen"
  friendly_name: "Kitchen Clock"
  feature_airq: "false"
  feature_wifi_page: "false"
  feature_weather: "false"
  feature_ble: "false"
  api_encryption_key: "kitchen_api_key_here"

packages:
  halo_v1_core: !include Halo-v1-Core.yaml
  airq_auto: !include packages/${airq_package_file}      # Loads airq-stubs.yaml
  ble_auto: !include packages/${ble_package_file}        # Loads ble-stubs.yaml
  wifi_auto: !include packages/${wifi_package_file}      # Loads wifi-stubs.yaml
  weather_auto: !include packages/${weather_package_file} # Loads weather-stubs.yaml
  lvgl_pages: !include packages/${lvgl_pages_file}       # Loads lvgl-pages-clock-only.yaml

wifi:
  ssid: !secret home_wifi_ssid
  password: !secret home_wifi_password

# ... rest of device config
```

---

## Managing Secrets for Multiple Devices

### Option 1: Shared Secrets File

**File:** `secrets.yaml`
```yaml
# WiFi networks
office_wifi_ssid: "OfficeNetwork"
office_wifi_password: "office_password_123"
home_wifi_ssid: "HomeNetwork"
home_wifi_password: "home_password_456"

# API keys (one per device for security)
office_api_key: "base64_key_for_office"
bedroom_api_key: "base64_key_for_bedroom"
kitchen_api_key: "base64_key_for_kitchen"
```

**In device files:**
```yaml
wifi_ssid: !secret office_wifi_ssid
api_encryption_key: !secret office_api_key
```

### Option 2: Per-Device Secrets

Each device can have its own `secrets.yaml` if preferred.

---

## Updating Multiple Devices

### Scenario 1: Update Core Hardware Config

**Change:** Fix a bug in `Halo-v1-Core.yaml`

**Steps:**
1. Edit `Halo-v1-Core.yaml`
2. Recompile all devices (they all use the updated core)

```bash
esphome compile halo-v1-79e384.yaml  # Office
esphome compile halo-v1-abc123.yaml  # Bedroom
esphome compile halo-v1-def456.yaml  # Kitchen
```

**Result:** All devices get the fix!

---

### Scenario 2: Add New Feature to One Device

**Change:** Add weather to the office device

**Steps:**
1. Edit `halo-v1-79e384.yaml`
2. Change `feature_weather: "false"` → `"true"`
3. Add `weather_entity_id: "weather.home"`
4. Recompile just that device

```bash
esphome compile halo-v1-79e384.yaml
esphome upload halo-v1-79e384.yaml --device office.local
```

**Result:**
- Office device now loads `weather-ha-actions.yaml` instead of `weather-stubs.yaml`
- Office device now loads `lvgl-pages-airq-wifi-weather-forecast.yaml` (3 features)
- Other devices unchanged!

---

### Scenario 3: Update a Package File

**Change:** Improve BLE tracking in `ble-core-simplified.yaml`

**Steps:**
1. Edit `packages/ble-core-simplified.yaml`
2. Recompile devices that use BLE

```bash
esphome compile halo-v1-abc123.yaml  # Bedroom (has BLE)
# Skip halo-v1-79e384.yaml (office - no BLE)
# Skip halo-v1-def456.yaml (kitchen - no BLE)
```

**Result:** Only bedroom device gets the BLE update (office and kitchen load `ble-stubs.yaml` so they're unaffected)

---

## Home Assistant Integration

### Device Naming

Each device appears in Home Assistant with its friendly name:

```yaml
# halo-v1-79e384.yaml
name: "halo-office"
friendly_name: "Office Monitor"
```

**In Home Assistant:**
- Entity names: `switch.office_monitor_auto_page_rotation`
- Device name: "Office Monitor"
- Integration: ESPHome (`halo-office`)

### Weather Entity Configuration

Different devices can use different weather entities:

```yaml
# Bedroom uses local weather
weather_entity_id: "weather.home"

# Living room uses forecast for vacation home
weather_entity_id: "weather.vacation_home"

# Office doesn't use weather (feature disabled)
```

---

## Best Practices for Multiple Devices

### 1. **Use Descriptive Names**
```yaml
# Good
name: "halo-office"
name: "halo-bedroom-main"
name: "halo-kids-room"

# Bad
name: "halo-1"
name: "halo-2"
```

### 2. **Document Each Device**

Add comments at the top of each device file:

```yaml
# ============================================================================
# Halo V1 - Office Device (MAC: 79:e3:84)
# ============================================================================
# Location: Main office desk
# Features: AirQ monitoring, WiFi info
# IP: 192.168.1.100 (static)
# Last updated: 2025-10-30
# ============================================================================
```

### 3. **Use Consistent Feature Flags**

Keep the same substitution names across all devices:

```yaml
# All devices use same flag names
feature_airq: "true"   # Not "enable_airq" on some devices
feature_ble: "false"   # Not "ble_enabled" on others
```

### 4. **Group Devices by Feature Set**

Organize your files:

```
├── minimal-devices/
│   ├── halo-v1-kitchen.yaml
│   └── halo-v1-bathroom.yaml
│
├── full-featured/
│   ├── halo-v1-office.yaml
│   └── halo-v1-living-room.yaml
│
└── bedroom-devices/
    ├── halo-v1-master-bedroom.yaml
    └── halo-v1-guest-bedroom.yaml
```

### 5. **Batch Compilation Script**

Create a helper script for updating all devices:

**compile-all.sh**
```bash
#!/bin/bash
echo "Compiling all Halo devices..."

esphome compile halo-v1-79e384.yaml  # Office
esphome compile halo-v1-abc123.yaml  # Bedroom
esphome compile halo-v1-def456.yaml  # Kitchen
esphome compile halo-v1-ghi789.yaml  # Living room

echo "All devices compiled!"
```

**upload-all.sh**
```bash
#!/bin/bash
echo "Uploading to all Halo devices..."

esphome upload halo-v1-79e384.yaml --device 192.168.1.100 &
esphome upload halo-v1-abc123.yaml --device 192.168.1.101 &
esphome upload halo-v1-def456.yaml --device 192.168.1.102 &
esphome upload halo-v1-ghi789.yaml --device 192.168.1.103 &

wait
echo "All devices updated!"
```

---

## Memory Considerations

### Planning Device Features

Use the memory budget to plan which features each device can have:

**BLE Compatible (< 110KB total):**
- ✅ Clock + Weather + BLE (~108KB) ← **Perfect for bedroom!**
- ✅ Clock + AirQ + BLE (~100KB) ← **Good for small rooms**
- ✅ Clock + WiFi + BLE (~95KB) ← **Good for monitoring**

**Non-BLE Setups:**
- Clock + AirQ + WiFi (~125KB) ← **Perfect for office!**
- Clock + AirQ + Weather (~130KB) ← **Good for kitchen**
- Clock + All features (~140KB) ← **Living room showcase**

### Example Device Allocation

| Room | Primary Need | Features | RAM | BLE? |
|------|-------------|----------|-----|------|
| Office | Air Quality | AirQ + WiFi | 125KB | No |
| Bedroom | Sleep Tracking | Weather + BLE | 108KB | Yes |
| Kitchen | Cooking Timer | Clock only | 45KB | Yes |
| Living Room | Showcase | All features | 140KB | No |
| Bathroom | Minimal | Clock + Weather | 105KB | Yes |

---

## Summary

### Key Advantages of Multi-Device Setup:

✅ **One codebase, many configurations** - All devices share core and packages
✅ **Independent feature selection** - Each device chooses what it needs
✅ **Automatic package loading** - Jinja2 handles the complexity
✅ **Easy updates** - Update core once, affects all devices
✅ **Selective updates** - Update packages, only affected devices recompile
✅ **Memory optimization** - Each device uses only what it needs
✅ **Flexible deployment** - Mix and match features per location

### The Magic:

Change **one line** in a device file:
```yaml
feature_weather: "false"  →  feature_weather: "true"
```

And automatically:
- Different package loads (`weather-stubs.yaml` → `weather-ha-actions.yaml`)
- Different pages load (`lvgl-pages-clock-only.yaml` → `lvgl-pages-weather-forecast-only.yaml`)
- Different memory footprint (~45KB → ~105KB)
- Same core, same packages directory!

**This is the power of Jinja2 dynamic configuration for multiple devices!**
