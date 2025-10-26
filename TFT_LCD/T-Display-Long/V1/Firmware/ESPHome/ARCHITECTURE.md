# Halo Firmware Architecture

## Overview

The Halo firmware uses a modular package-based architecture that allows each device to include only the features it needs. This document explains how the system works and what packages are available.

## Architecture Diagram

```
Device Configuration File (e.g., halo-v1-79e384.yaml)
├── Utility Packages (Optional)
│   ├── memory-stats.yaml
│   ├── wifi-display.yaml
│   └── wireguard.yaml + wireguard-display.yaml + time-update-wireguard.yaml
│
├── Feature Packages (Choose core OR stubs for each)
│   ├── AirQ: airq-core.yaml + airq-display.yaml OR airq-stubs.yaml
│   ├── Weather: weather-core.yaml + weather-sensors.yaml + weather-fonts-text.yaml OR weather-stubs.yaml
│   ├── WiFi Monitoring: wifi-core.yaml OR wifi-stubs.yaml
│   └── BLE: ble-core.yaml OR ble-stubs.yaml
│
├── LVGL Pages Package (Choose ONE)
│   ├── lvgl-pages-full.yaml
│   ├── lvgl-pages-airq-weather.yaml
│   ├── lvgl-pages-airq-only.yaml
│   ├── lvgl-pages-weather.yaml
│   └── lvgl-pages-clock-only.yaml
│
├── Core Configuration (Always include)
│   └── Halo-v1-Core.yaml
│       ├── page-registry.yaml
│       ├── vertical-clock-core.yaml
│       └── time-update-basic.yaml
│
└── Device-Specific Overrides
    ├── Default Page selector
    ├── LVGL buffer_size
    └── Other custom settings
```

## Package Categories

### 1. Core Packages (Always Included)

These are loaded automatically when you import `Halo-v1-Core.yaml`:

| Package | Purpose |
|---------|---------|
| `page-registry.yaml` | Page rotation management system |
| `vertical-clock-core.yaml` | Vertical clock display (core feature) |
| `time-update-basic.yaml` | Time synchronization (basic, no WireGuard) |

**Location:** Included in `Halo-v1-Core.yaml`

**When to modify:** Never - these are essential for all devices

### 2. Utility Packages (Optional System Features)

Add these to your device file as needed:

| Package | Purpose | Recommended |
|---------|---------|-------------|
| `memory-stats.yaml` | Memory usage monitoring and diagnostics | Yes (dev/debug) |
| `wifi-display.yaml` | WiFi connection info overlay on display | Yes |
| `wireguard.yaml` | WireGuard VPN client | Optional |
| `wireguard-display.yaml` | WireGuard status display | With WireGuard |
| `time-update-wireguard.yaml` | Advanced time sync (overrides basic) | With WireGuard |

**Location:** Include in device configuration file

**Memory Impact:** Minimal (~1-2% buffer each)

### 3. Feature Packages (Core + Stubs Pattern)

For each feature, include EITHER the core package OR the stubs package (never both):

#### AirQ - Air Quality Monitoring
- **Enable:** `airq-core.yaml` + `airq-display.yaml`
- **Disable:** `airq-stubs.yaml`
- **Memory Impact:** Moderate (~3-4% buffer)
- **Hardware Required:** SCD4x, MICS-4514, BME280, or SEN5x sensors

#### Weather - Forecasting
- **Enable:** `weather-core.yaml` + `weather-sensors.yaml` + `weather-fonts-text.yaml`
- **Disable:** `weather-stubs.yaml`
- **Memory Impact:** High (~5-7% buffer) - includes fonts and multiple pages
- **Dependencies:** Home Assistant weather integration

#### WiFi Monitoring - Network Statistics Page
- **Enable:** `wifi-core.yaml`
- **Disable:** `wifi-stubs.yaml`
- **Memory Impact:** Low (~1-2% buffer)
- **Note:** Different from `wifi-display.yaml` (which shows connection info)

#### BLE - Bluetooth Low Energy
- **Enable:** `ble-core.yaml`
- **Disable:** `ble-stubs.yaml` ⭐ **RECOMMENDED**
- **Memory Impact:** Very High (~10-15% buffer)
- **Warning:** Can cause heap exhaustion and crashes!

### 4. LVGL Pages Packages

Choose ONE package that matches your enabled features:

| Package | Features Included | Use Case |
|---------|------------------|----------|
| `lvgl-pages-full.yaml` | Clock, AirQ, WiFi, Weather (all) | Full-featured device |
| `lvgl-pages-airq-weather.yaml` | Clock, AirQ, Weather | AirQ + Weather combo |
| `lvgl-pages-airq-only.yaml` | Clock, AirQ | Air quality monitor |
| `lvgl-pages-weather.yaml` | Clock, Weather | Weather station |
| `lvgl-pages-clock-only.yaml` | Clock only | Minimal display |

**Important:** LVGL pages package MUST match your enabled features!

## Package Loading Order

Packages are loaded in this order:

```yaml
1. Utility packages (memory_stats, wifi_display, wireguard)
2. Feature packages (airq, weather, wifi, ble) OR stubs
3. LVGL pages package
4. Core configuration (Halo-v1-Core.yaml)
5. Device-specific overrides
```

This order ensures:
- Features load before Core to avoid conflicts
- Core can reference feature globals (provided by core or stubs)
- Device overrides take precedence over Core defaults

## Configuration Patterns

### Pattern 1: Minimal Device (Clock Only)

```yaml
packages:
  # No utilities needed

  # All features disabled
  airq_stubs: !include packages/airq-stubs.yaml
  weather_stubs: !include packages/weather-stubs.yaml
  wifi_stubs: !include packages/wifi-stubs.yaml
  ble_stubs: !include packages/ble-stubs.yaml

  # Clock-only pages
  lvgl_pages: !include packages/lvgl-pages-clock-only.yaml

  # Core
  core: !include Halo-v1-Core.yaml

lvgl:
  buffer_size: 35%  # Maximum available
```

### Pattern 2: Single Feature Device (AirQ)

```yaml
packages:
  # Utilities
  memory_stats: !include packages/memory-stats.yaml
  wifi_display: !include packages/wifi-display.yaml

  # AirQ enabled
  airq_core: !include packages/airq-core.yaml
  airq_display: !include packages/airq-display.yaml

  # Other features disabled
  weather_stubs: !include packages/weather-stubs.yaml
  wifi_stubs: !include packages/wifi-stubs.yaml
  ble_stubs: !include packages/ble-stubs.yaml

  # AirQ pages
  lvgl_pages: !include packages/lvgl-pages-airq-only.yaml

  # Core
  core: !include Halo-v1-Core.yaml

lvgl:
  buffer_size: 30%
```

### Pattern 3: Multi-Feature Device (AirQ + Weather)

```yaml
packages:
  # Utilities
  memory_stats: !include packages/memory-stats.yaml
  wifi_display: !include packages/wifi-display.yaml

  # AirQ enabled
  airq_core: !include packages/airq-core.yaml
  airq_display: !include packages/airq-display.yaml

  # Weather enabled
  weather_core: !include packages/weather-core.yaml
  weather_sensors: !include packages/weather-sensors.yaml
  weather_fonts_text: !include packages/weather-fonts-text.yaml

  # Other features disabled
  wifi_stubs: !include packages/wifi-stubs.yaml
  ble_stubs: !include packages/ble-stubs.yaml

  # AirQ + Weather pages
  lvgl_pages: !include packages/lvgl-pages-airq-weather.yaml

  # Core
  core: !include Halo-v1-Core.yaml

lvgl:
  buffer_size: 27%
```

### Pattern 4: Full-Featured Device

```yaml
packages:
  # Utilities
  memory_stats: !include packages/memory-stats.yaml
  wifi_display: !include packages/wifi-display.yaml
  wireguard: !include packages/wireguard.yaml
  wireguard_display: !include packages/wireguard-display.yaml
  time_update_wireguard: !include packages/time-update-wireguard.yaml

  # All features enabled
  airq_core: !include packages/airq-core.yaml
  airq_display: !include packages/airq-display.yaml
  weather_core: !include packages/weather-core.yaml
  weather_sensors: !include packages/weather-sensors.yaml
  weather_fonts_text: !include packages/weather-fonts-text.yaml
  wifi_core: !include packages/wifi-core.yaml
  ble_stubs: !include packages/ble-stubs.yaml  # Still disabled!

  # Full pages
  lvgl_pages: !include packages/lvgl-pages-full.yaml

  # Core
  core: !include Halo-v1-Core.yaml

lvgl:
  buffer_size: 25%
```

## Memory Optimization

### Buffer Size Guidelines

| Configuration | Buffer Size | Available Features |
|---------------|-------------|-------------------|
| Clock only | 30-35% | Minimal |
| Clock + AirQ | 28-32% | Single feature |
| Clock + Weather | 25-28% | Single feature (heavy) |
| Clock + AirQ + Weather | 26-28% | Two features |
| Full featured (no BLE) | 25% | All features |
| With BLE enabled | 15-20% | ⚠️ Not recommended |

### Memory Usage by Package

| Package Type | Typical Impact | Notes |
|-------------|---------------|-------|
| Core | ~15-20% | Always required |
| Utilities | ~1-2% each | Minimal impact |
| AirQ | ~3-4% | Moderate sensors |
| Weather | ~5-7% | Includes fonts + pages |
| WiFi monitoring | ~1-2% | Single page |
| BLE | ~10-15% | ⚠️ Causes instability |

## Best Practices

1. **Start minimal, add features** - Begin with clock-only, add features as needed
2. **Always include stubs** - Every feature needs either core OR stubs
3. **Match pages to features** - LVGL pages package must align with enabled features
4. **Monitor memory** - Include `memory-stats.yaml` during development
5. **Avoid BLE** - Unless absolutely required, use `ble-stubs.yaml`
6. **Test incrementally** - Add one feature at a time, test, then add more
7. **Document your config** - Use comments to explain your device's purpose

## Common Issues

### Issue: Build fails with "undefined reference to page_rotation_*"
**Cause:** Missing stub package for a disabled feature
**Solution:** Include the stub package (e.g., `weather-stubs.yaml`)

### Issue: Heap exhaustion / crashes
**Cause:** Too many features or buffer_size too high
**Solution:** Reduce buffer_size or disable unneeded features

### Issue: Wrong pages showing on display
**Cause:** LVGL pages package doesn't match enabled features
**Solution:** Use correct pages package (e.g., `lvgl-pages-airq-only.yaml` for AirQ-only)

### Issue: Duplicate package errors
**Cause:** Same package included in both device file and Core
**Solution:** Remove from device file (Core doesn't include optional features anymore)

## File Structure

```
TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/
├── Halo-v1-Core.yaml                    # Core configuration (minimal)
├── halo-v1-79e384.yaml                  # Template device config
├── halo-v1-airq-weather-example.yaml    # Example: AirQ + Weather
├── ARCHITECTURE.md                       # This file
├── MIGRATION-GUIDE.md                   # Migration instructions
└── packages/
    ├── Core packages
    │   ├── page-registry.yaml
    │   ├── vertical-clock-core.yaml
    │   └── vertical-clock-display.yaml
    ├── Utility packages
    │   ├── memory-stats.yaml
    │   ├── wifi-display.yaml
    │   ├── wireguard.yaml
    │   ├── wireguard-display.yaml
    │   ├── time-update-basic.yaml
    │   └── time-update-wireguard.yaml
    ├── AirQ packages
    │   ├── airq-core.yaml
    │   ├── airq-display.yaml
    │   └── airq-stubs.yaml
    ├── Weather packages
    │   ├── weather-core.yaml
    │   ├── weather-sensors.yaml
    │   ├── weather-fonts-text.yaml
    │   ├── weather-stubs.yaml
    │   └── weather-page-*.yaml (8 files)
    ├── WiFi packages
    │   ├── wifi-core.yaml
    │   ├── wifi-stubs.yaml
    │   └── lvgl-wifi-*.yaml
    ├── BLE packages
    │   ├── ble-core.yaml
    │   └── ble-stubs.yaml
    ├── LVGL pages packages
    │   ├── lvgl-pages-full.yaml
    │   ├── lvgl-pages-airq-weather.yaml
    │   ├── lvgl-pages-airq-only.yaml
    │   ├── lvgl-pages-weather.yaml
    │   └── lvgl-pages-clock-only.yaml
    └── Documentation
        ├── README-DEVICE-CONFIGURATION.md
        ├── README-AIRQ-INTEGRATION.md
        ├── README-BLE-INTEGRATION.md
        └── README-WIFI-INTEGRATION.md
```

## Further Reading

- **[README-DEVICE-CONFIGURATION.md](packages/README-DEVICE-CONFIGURATION.md)** - Complete device config guide with examples
- **[MIGRATION-GUIDE.md](MIGRATION-GUIDE.md)** - Migrating from old architecture
- **[halo-v1-79e384.yaml](halo-v1-79e384.yaml)** - Commented template for new devices
