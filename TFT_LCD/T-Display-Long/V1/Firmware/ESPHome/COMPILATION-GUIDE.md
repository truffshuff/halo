# ESPHome Compilation Guide for Halo V1

This guide explains how to compile your Halo device configuration using either the ESPHome CLI or Home Assistant ESPHome dashboard.

## Problem: GitHub URL Packages Don't Work

ESPHome's package loader has a bug where it treats package labels as component names when loading from GitHub URLs, causing errors like:

```
Component not found: pkg_a
```

This happens because ESPHome evaluates package labels **before** fetching files from GitHub.

## Solution: Two Compilation Methods

## ⚠️ IMPORTANT: Jinja2 Limitations

**Jinja2 substitutions in `!include` paths only work with ESPHome CLI!**

The Home Assistant ESPHome dashboard's YAML parser evaluates `!include` statements **before** processing Jinja2 substitutions, causing errors like:

```
Error reading file /config/esphome/packages/${airq_package_file}
```

**What this means:**
- ✅ **ESPHome CLI**: Full Jinja2 dynamic configuration works perfectly
- ❌ **HA Dashboard**: Must manually comment/uncomment packages

---

### Method 1: Local Compilation (ESPHome CLI) - RECOMMENDED ⭐

**Advantages:**
- Works perfectly with `!include` syntax
- No file copying required
- Faster compilation
- Full Jinja2 dynamic package selection

**Requirements:**
- ESPHome CLI installed locally
- Clone of this repository

**Steps:**

1. Navigate to the firmware directory:
```bash
cd /path/to/halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome
```

2. Compile using the local file:
```bash
esphome compile halo-v1-79e384.yaml
```

3. Flash to device:
```bash
esphome upload halo-v1-79e384.yaml
```

**File to use:** `halo-v1-79e384.yaml` (uses `!include` for local paths)

---

### Method 2: Home Assistant ESPHome Dashboard

**Advantages:**
- Web-based interface
- No CLI required
- Integrated with Home Assistant

**Disadvantages:**
- ❌ **No Jinja2 dynamic configuration** - Must manually edit packages
- Requires copying files to `/config/esphome/` directory
- More setup steps
- More error-prone (easy to forget to update pages package)

**Steps:**

#### A. Copy Files to Home Assistant

You need to copy the following files from your GitHub repository to your Home Assistant's `/config/esphome/` directory:

**Core files:**
```
Halo-v1-Core.yaml
halo-v1-79e384-ha-manual.yaml  (manual package selection for HA dashboard)
```

⚠️ **Use the `-ha-manual.yaml` version** which has manual package selection since Jinja2 doesn't work in HA dashboard!

**Packages directory** (entire directory with all files):
```
packages/
├── airq-display.yaml
├── airq-stubs.yaml
├── ble-core-simplified.yaml
├── ble-stubs.yaml
├── lvgl-pages-clock-only.yaml
├── lvgl-pages-airq-only.yaml
├── (all other lvgl-pages-*.yaml files)
├── page-registry.yaml
├── psram-helpers.yaml
├── time-update-basic.yaml
├── vertical-clock-core.yaml
├── weather-ha-actions.yaml
├── weather-stubs.yaml
├── wifi-display.yaml
├── wifi-stubs.yaml
├── wireguard.yaml
└── wireguard-stubs.yaml
```

**Methods to copy files:**

**Option 1 - Using File Editor Add-on:**
1. Install "File Editor" add-on in Home Assistant
2. Navigate to `/config/esphome/`
3. Create `packages/` folder
4. Upload files one by one

**Option 2 - Using Samba/SMB Share:**
1. Install "Samba share" add-on in Home Assistant
2. Connect to `\\homeassistant\config\esphome\` from your computer
3. Copy entire `packages/` folder and core YAML files

**Option 3 - Using SSH/Terminal:**
```bash
# From your local machine
scp -r packages/ root@homeassistant:/config/esphome/
scp Halo-v1-Core.yaml root@homeassistant:/config/esphome/
scp halo-v1-79e384.yaml root@homeassistant:/config/esphome/
```

#### B. Compile from Dashboard

1. Open Home Assistant → ESPHome Dashboard
2. Your device (`halo-v1-79e384`) should appear in the list
3. Click "Install" → "Compile"
4. Wait for compilation to complete
5. Flash to device using your preferred method

---

## File Structure After Setup

After copying files, your Home Assistant `/config/esphome/` directory should look like:

```
/config/esphome/
├── halo-v1-79e384.yaml          # Your device config
├── Halo-v1-Core.yaml            # Core hardware config
├── .esphome/                    # ESPHome cache (auto-created)
└── packages/
    ├── airq-display.yaml
    ├── airq-stubs.yaml
    ├── ble-core-simplified.yaml
    ├── ble-stubs.yaml
    ├── lvgl-pages-*.yaml        # All page configuration files
    ├── page-registry.yaml
    ├── psram-helpers.yaml
    ├── time-update-basic.yaml
    ├── vertical-clock-core.yaml
    ├── weather-ha-actions.yaml
    ├── weather-stubs.yaml
    ├── wifi-display.yaml
    ├── wifi-stubs.yaml
    ├── wireguard.yaml
    └── wireguard-stubs.yaml
```

---

## Dynamic Configuration with Jinja2

Both methods support the revolutionary Jinja2 automatic package selection!

### How to Enable/Disable Features

Edit `halo-v1-79e384.yaml` and change feature flags from `"false"` to `"true"`:

```yaml
substitutions:
  # Display Features
  feature_airq: "true"              # Enable Air Quality page
  feature_wifi_page: "false"        # Disable WiFi page
  feature_wireguard: "false"        # Disable WireGuard VPN

  # Weather Features (NEW!)
  feature_weather: "true"           # Enable weather via HA Actions!
  weather_entity_id: "weather.home" # Your HA weather entity

  # Connectivity
  feature_ble: "true"               # Enable BLE presence detection
```

The configuration **automatically**:
- Loads `airq-display.yaml` (feature_airq = true)
- Loads `weather-ha-actions.yaml` (feature_weather = true)
- Loads `ble-core-simplified.yaml` (feature_ble = true)
- Loads `wifi-stubs.yaml` (feature_wifi_page = false)
- Loads `wireguard-stubs.yaml` (feature_wireguard = false)
- Selects the correct LVGL pages package based on enabled features

**No manual editing of package imports required!** Just change the flags and recompile.

---

## Memory Budget Guide

ESP32-S3 has ~210KB internal RAM available. BLE requires ~60KB, leaving ~110KB for everything else.

### BLE Compatible (< 110KB total):
- ✅ Clock only: ~45KB
- ✅ Clock + AirQ: ~65KB
- ✅ Clock + WiFi: ~85KB
- ✅ **Clock + Weather (HA Actions): ~105KB** ← NEW! BLE compatible!
- ✅ **Clock + BLE + Weather: ~108KB** ← Amazing!

### NOT BLE Compatible (> 110KB total):
- ⚠️ Clock + AirQ + WiFi: ~125KB
- ⚠️ Clock + AirQ + Weather: ~130KB
- ⚠️ Clock + WiFi + Weather: ~135KB
- ⚠️ Clock + AirQ + WiFi + Weather: ~140KB

**Why is Weather different now?**

Old approach: Weather sensors consumed ~75KB (125+ sensor entities)
New approach: Weather via HA Actions consumes ~3KB (on-demand data fetching)

This **70KB savings** enables BLE + Weather coexistence!

---

## Troubleshooting

### Error: "Component not found: pkg_a" (or similar)

**Cause:** Trying to use GitHub URL packages in HA dashboard
**Solution:** Use Method 2 and copy files locally, or use Method 1 (CLI)

### Error: "File not found: Halo-v1-Core.yaml"

**Cause:** Core file not in `/config/esphome/` directory
**Solution:** Copy `Halo-v1-Core.yaml` to `/config/esphome/`

### Error: "Couldn't find ID 'page_rotation_AirQ_enabled'"

**Cause:** Missing stub package files
**Solution:** Ensure all `*-stubs.yaml` files are copied to `/config/esphome/packages/`

### Compilation succeeds but device doesn't work

**Cause:** Might be using wrong page configuration
**Solution:** Check that `lvgl_pages_file` substitution matches your enabled features

---

## Updating Configuration

### After Changing Feature Flags:

1. Edit `halo-v1-79e384.yaml`
2. Change desired feature flags
3. Recompile using your preferred method
4. Flash to device

The Jinja2 substitutions will automatically select the correct packages!

### After Updating Repository:

**Method 1 (CLI):**
```bash
git pull
esphome compile halo-v1-79e384.yaml
```

**Method 2 (HA Dashboard):**
Re-copy updated files to `/config/esphome/`:
```bash
scp -r packages/ root@homeassistant:/config/esphome/
scp Halo-v1-Core.yaml root@homeassistant:/config/esphome/
```

---

## Why Not Use GitHub URLs Directly?

We tried! ESPHome has a bug where package labels are evaluated as component names before fetching from GitHub. This causes errors like:

```
Component not found: page_registry
Component not found: pkg_a
```

Until this is fixed in ESPHome, local `!include` files are the only working solution.

---

## Summary

| Method | Complexity | Speed | Jinja2 Support | Recommended |
|--------|-----------|-------|----------------|-------------|
| **CLI (Local)** | Easy | Fast | ✅ **YES** - Full automatic config | ✅ **STRONGLY RECOMMENDED** |
| **HA Dashboard** | Medium | Medium | ❌ **NO** - Manual package editing | ⚠️ Only if web UI required |

**Best practice:** Use ESPHome CLI for compilation. The automatic Jinja2 configuration is worth it!

**If you must use HA dashboard:** Use `halo-v1-79e384-ha-manual.yaml` and manually comment/uncomment packages.
