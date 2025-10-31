# How to Compile Halo from Home Assistant Dashboard

## The Problem

The **halo-v1-79e384-ha-github.yaml** file doesn't work because:
- It tries to load all packages from GitHub URLs
- The core configuration (Halo-v1-Core.yaml) contains nested `!include` statements
- ESPHome can't resolve nested includes from GitHub URLs
- Missing components: `display:`, `light:`, `i2c:`, `api:`, etc.

## The Solution: Use Local Files

### Step 1: Copy Files to Home Assistant

Copy these files/folders to your Home Assistant ESPHome directory (`/config/esphome/`):

```bash
# From your computer to Home Assistant
# (Adjust paths based on your HA setup - SSH, SAMBA, etc.)

# Copy core file
cp Halo-v1-Core.yaml /path/to/homeassistant/config/esphome/

# Copy entire packages directory
cp -r packages/ /path/to/homeassistant/config/esphome/

# Copy device file
cp halo-v1-79e384-ha-manual.yaml /path/to/homeassistant/config/esphome/
```

**OR** if you have Home Assistant File Editor or Samba access:
1. Open File Editor add-on in Home Assistant
2. Navigate to `/config/esphome/`
3. Upload Halo-v1-Core.yaml
4. Upload entire packages/ folder
5. Upload halo-v1-79e384-ha-manual.yaml

### Step 2: Compile from HA Dashboard

1. Open Home Assistant
2. Go to **Settings** → **Add-ons** → **ESPHome**
3. Open ESPHome dashboard
4. Click **"+ NEW DEVICE"** or edit existing device
5. Select **halo-v1-79e384-ha-manual.yaml**
6. Click **INSTALL**

## File You Should Use

✅ **halo-v1-79e384-ha-manual.yaml** - This works with HA dashboard!

❌ **halo-v1-79e384-ha-github.yaml** - This doesn't work (missing components)

## Why Manual File Works

The **halo-v1-79e384-ha-manual.yaml** file:
- Uses `!include Halo-v1-Core.yaml` to load complete core configuration
- Includes all required components: `display:`, `light:`, `i2c:`, `api:`, etc.
- Uses local `!include` for packages (works with HA dashboard)
- Properly documented with feature enable/disable instructions

## Alternative: Compile from CLI

If you prefer to use the GitHub repository directly:

```bash
# Clone repo
git clone https://github.com/truffshuff/halo.git
cd halo/TFT_LCD/T-Display-Long/V1/Firmware/ESPHome

# Compile with ESPHome CLI
esphome compile halo-v1-79e384-ha-manual.yaml

# Upload to device
esphome upload halo-v1-79e384-ha-manual.yaml
```

## Summary

**Bottom line:** The GitHub URL approach doesn't work because of nested package limitations. Use local files with `!include` instead.
