# ESPHome Changelog Implementation Recommendations for Halo v1

## ESPHome 2026.11.0 Implementation (November 23, 2026)

**Upgraded Project Version**: 2026.11.0 (from 2026.10.0)  
**Hardware**: ESP32-S3 (LilyGo T-Display-Long)  
**Changelog Reference**: https://esphome.io/changelog/2026.11.0/

### Executive Summary (2026.11.0)

ESPHome 2026.11.0 introduces framework-wide memory optimizations, C++ compiled automations, and resilient UDP-fallback OTA updates.

#### Key Changes Implemented for Halo v1:
1. **Critical Behavioral Fix — Non-Blocking `http_request` in Scripts:**
   - In 2026.11.0, `http_request` actions in scripts and action blocks execute asynchronously (non-blocking).
   - In `packages/features/weather/weather_base.yaml`, moved `switch.turn_on: ble_scanner_switch` and `weather_fetch_lock = false` inside `on_response:` and `on_error:` callbacks to prevent premature lock release and scanner reactivation while the 57 KB payload is still downloading.
   - In `packages/system/system_management.yaml`, sequenced the Google (`google.com/generate_204`) and Cloudflare (`1.1.1.1`) reachability checks via `diag_check_cloudflare_script` and updated `binary_sensor.template.publish` calls to publish from response callbacks upon actual completion.
2. **Resilient OTA (UDP Fallback):**
   - Native OTA uploads now leverage UDP on port 6053 with automatic 15-second fallback to TCP, directly alleviating 2.4 GHz WiFi lockstep TCP stalls during wireless flashing.
3. **Internal SRAM Optimizations:**
   - Automations are compiled directly down to plain C++, and entity name strings/metadata shift to flash across platforms, reducing static internal SRAM pressure on the ESP32-S3.
4. **Early Boot Log Replay:**
   - Pre-connection boot logs are buffered in memory and replayed once Home Assistant or an API logger connects.
5. **Version Bumps & Docs:**
   - `min_version` bumped to `2026.11.0` in `Halo-v1.yaml` and `halo-v1-79e384.yaml`.
   - Updated documentation in `docs/ARCHITECTURE.md` and `docs/MEMORY.md`.

---

# Historical: ESPHome 2026.5.0 & 2026.6.0 Recommendations

**Analysis Date**: July 7, 2026  
**Current Project Version**: 2026.4.0  
**Hardware**: ESP32-S3 (LilyGo T-Display-Long)  
**Key Features**: LVGL Display, BLE, OTA, Air Quality Sensors, Weather Integration

---

## Executive Summary

Your Halo project can benefit significantly from upgrades in **2026.5.0 and 2026.6.0**. The most impactful changes for your use case are:

1. **LVGL performance improvements** (4.5× faster config validation)
2. **New display metadata system** for automatic byte order selection
3. **BLE reliability fixes** (critical if using BLE proxy)
4. **Audio stack modernization** (if adding sound effects/alerts)
5. **Configuration tooling** (YAML frontmatter, build flags)
6. **Performance & memory optimizations** for ESP32-S3

---

## 🔴 CRITICAL/HIGH PRIORITY

### 1. Upgrade to ESPHome 2026.6.0 (from 2026.4.0)

**Why**: Major stability and performance improvements specific to ESP32-S3 and LVGL

**Action Required**:
```yaml
esphome:
  min_version: 2026.6.0  # Update from 2026.4.0
```

**Benefits**:
- LVGL config validation: **210 ms → 17 ms** (12× faster) on subsequent validations
- Reduced CPU utilization on main loop
- Critical BLE reliability fixes for proxy setups
- Better memory management with WiFi/Ethernet disabled at boot

**Estimated Impact**: ⭐⭐⭐⭐⭐ (Major)

---

### 2. Implement LVGL Display Metadata (2026.6.0 Feature)

**Current**: Manual `byte_order` and rotation configuration needed  
**New**: Display drivers now auto-register metadata

**Action**:
Update your LVGL display configuration to leverage new metadata system:

```yaml
display:
  - platform: mipi_spi  # or your display driver
    # New in 2026.6.0: metadata is automatically provided by driver
    # - byte_order: automatically selected
    # - rotation: automatically applied
    # - draw_rounding: merged from requirements
    # - has_writer: auto-detected
```

**Benefits**:
- Eliminates manual byte-order configuration errors
- Mixed display byte orders rejected at validation (prevents silent bugs)
- LVGL can auto-optimize drawing path

**Estimated Impact**: ⭐⭐⭐ (Medium - Eliminates manual config)

**Changelog Reference**: 2026.6.0 - "Display metadata for LVGL validation"

---

### 3. Fix BLE Reliability for Bluetooth Proxy (2026.5.0 Feature)

**Issue**: `status=0x85` (133) GATT failures under sustained WiFi traffic  
**Status**: Fixed in 2026.5.0 ([#16036](https://github.com/esphome/esphome/pull/16036))

**Action**:
No code changes needed—upgrade handles automatically. This fix tracks `CONNECTED` and `ESTABLISHED` BLE clients and holds `ESP_COEX_PREFER_BT` for the connection lifetime.

**Before/After Improvement**:
- Heavy WiFi traffic: `status=133` timeout after 19 s → `status=0` success in 228 ms
- Directly affects Yale/August lock reliability and other BLE client operations

**Estimated Impact**: ⭐⭐⭐⭐ (Critical if using BLE proxy/clients)

**Changelog Reference**: 2026.5.0 - "BLE Reliability Fix for Bluetooth Proxies"

---

## 🟡 MEDIUM PRIORITY

### 4. Add YAML Frontmatter for Project Metadata (2026.6.0 Feature)

**What's New**: Leading `---`-separated YAML document for opaque metadata

**Current**: No metadata support  
**New**: Can add version, author, labels at config level

**Action**:
Add frontmatter to `Halo-v1.yaml`:

```yaml
---
author: "yashmulgaonkar"
version: "26.04.25.2142"
labels:
  - "air-quality"
  - "display"
  - "esp32-s3"
---

substitutions:
  # ... rest of config
```

**Benefits**:
- Version tracking in YAML (integrated with Device Builder)
- Labels for organizing multi-device setups
- Future tooling can read metadata (Device Builder)
- Human-readable project documentation

**Estimated Impact**: ⭐⭐ (Nice to have - organizational benefit)

**Changelog Reference**: 2026.6.0 - "YAML frontmatter"

---

### 5. Implement `esphome.build_flags` for Compiler Options (2026.6.0 Feature)

**Current Limitation**: PlatformIO-only build_flags  
**New**: Top-level `esphome.build_flags` applies to both PlatformIO and native ESP-IDF

**Action**:
If you have compiler optimizations, migrate to the new unified approach:

```yaml
esphome:
  name: halo-v1
  friendly_name: Halo Air Quality Sensor
  build_flags:  # NEW in 2026.6.0 - applies to both PlatformIO and IDF
    - -DCONFIG_FREERTOS_TICK_RATE_HZ=1000
    # Any other compiler flags
```

**Benefits**:
- Unified flag handling across build systems
- Future-proofs for native ESP-IDF toolchain option
- Works with both PlatformIO (current) and native IDF

**Estimated Impact**: ⭐⭐ (Low for current setup, medium if future native-IDF migration)

**Changelog Reference**: 2026.6.0 - "`esphome.build_flags` for IDF + PlatformIO"

---

### 6. Enhanced OTA Capabilities (2026.5.0 Features)

**New OTA Capabilities**:
- Partition table updates (convert from Tasmota → ESPHome over OTA)
- Bootloader updates
- Soft-brick recovery via factory partition
- Web server OTA as alternative to native API

**Current Config** (from your YAML):
```yaml
ota:
  - platform: esphome
    id: ota_default
    # ... callbacks
```

**Optional Enhancements**:
If you want to enable partition table updates:

```yaml
ota:
  - platform: esphome
    id: ota_default
    allow_partition_access: true  # NEW in 2026.5.0
    # Allows converting devices from other firmwares
    # Verify new partition table, sanity checks, commit only if boot success
```

**Benefits**:
- Can re-flash firmware even if stuck in broken state
- No USB cable required for soft-brick recovery
- Partition table updates for multi-firmware migrations

**Estimated Impact**: ⭐⭐⭐ (Operational resilience)

**Changelog Reference**: 2026.5.0 - "OTA Platform Enhancements"

---

### 7. Main Loop and Watchdog Optimization (2026.5.0 Feature)

**What Improved**:
- Every component's `loop()` now runs at configured `loop_interval_` cadence (default ~62 Hz)
- Watchdog feed rate corrected (1000× cost regression from 2019 bug)
- Background events (WiFi RX, BLE, etc.) still wake components within one tick

**Action**: No code required for basic benefit. To leverage power savings:

If you have custom components with tight loops, ensure they use:
```cpp
// For real-time components needing fast wakes
HighFrequencyLoopRequester loop_req;
```

**Benefits**:
- **Power savings**: 2.0 mA → 1.1 mA current measured (45% reduction on OpenThread proof-of-concept)
- **CPU efficiency**: ~41 ms per minute CPU reclaimed on typical ESP32
- **Reduced watchdog overhead**: 46.5 ms → 21.6 ms per 60s window

**Estimated Impact**: ⭐⭐⭐ (Power efficiency, especially important for battery devices)

**Changelog Reference**: 2026.5.0 - "Main Loop and Watchdog Architecture Overhaul"

---

## 🟢 LOW PRIORITY / CONDITIONAL

### 8. Audio Stack Modernization (2026.5.0 & 2026.6.0)

**Applies If**: You want to add sound effects, voice alerts, or notification sounds

**New Components**:
- `audio_http`: Play audio from arbitrary HTTP URLs
- `router` speaker: Switch between output speakers at runtime
- Zero-copy ring buffers (eliminated staging buffer & memmove)
- Any-bit-depth audio mixing (8, 16, 24, 32-bit)

**Current Status**: Your project doesn't appear to have audio output

**If Adding Audio**:
```yaml
# Example: HTTP audio source + speaker
media_player:
  - platform: speaker
    id: my_speaker
    speaker: my_i2s_speaker

audio_http:
  # NEW in 2026.5.0

# Then in automations:
on_click:
  - media_player.play_media:
      media_url: "https://example.com/notification.mp3"
```

**Estimated Impact**: ⭐ (Not applicable unless adding sound)

**Changelog Reference**: 2026.5.0 - "Audio Stack Modernization" & 2026.6.0 - "Audio Stack Modernization"

---

### 9. New Components & Hardware Support (2026.6.0)

**New IMU Component** - Motion Hub:
- Bosch BMI270 ([#16202](https://github.com/esphome/esphome/pull/16202))
- STMicro LSM6DS3TR-C ([#16232](https://github.com/esphome/esphome/pull/16232))

**New Audio DACs**:
- PCM5122 Texas Instruments I2S DAC ([#15709](https://github.com/esphome/esphome/pull/15709))

**New Sensors**:
- XDB401 I2C pressure sensor ([#15108](https://github.com/esphome/esphome/pull/15108))

**USB-Serial Drivers**:
- FTDI FT23XX family ([#14587](https://github.com/esphome/esphome/pull/14587))
- Prolific PL2303 family ([#16885](https://github.com/esphome/esphome/pull/16885))

**Your Halo Currently Has**:
- CO2: SCD40/SCD41 ✓
- Particulate/VOC/NOx: SEN55 ✓
- Multi-gas: MICS-4514 ✓
- Pressure/Temp/Humidity: BME280 ✓
- Power: SY6970 PMIC

**Opportunities**:
- Could add BMI270 IMU for orientation-aware display
- Could add USB-serial support if using external devices
- Current sensor coverage is excellent

**Estimated Impact**: ⭐ (Not needed unless expanding hardware)

**Changelog Reference**: 2026.6.0 - "New Components and Hardware Support"

---

### 10. Sendspin Multi-Room Audio (2026.5.0)

**Applies If**: Building multi-room synchronized audio system

**Your Project**: Single device (not applicable)

**Estimated Impact**: ⭐ (Not applicable)

---

### 11. Zigbee on ESP32 (2026.5.0)

**New Feature**: Native Zigbee support on ESP32-C6 and ESP32-H2

**Your Hardware**: ESP32-S3 (not applicable - S3 doesn't have Zigbee radio)

**Estimated Impact**: ⭐ (Hardware limitation)

---

## 🔧 BREAKING CHANGES TO VERIFY

### From 2026.5.0:
- ✅ **Modbus Server Split**: Not using Modbus → No action
- ✅ **API max_connections**: Default 8→5 on ESP32. If using >5 API connections, set explicitly
- ✅ **OTA Platform Consolidation**: Only use single `ota: - platform: esphome` entry
- ✅ **Audio WAV Codec**: If using audio, add `audio: codecs: wav:` if playing WAV-only files
- ✅ **FloatOutput Power Scaling**: Not using custom outputs → No action

### From 2026.6.0:
- ✅ **ESP8266 WiFi Security**: Only affecting ESP8266 (not your S3)
- ✅ **custom_components/ Folder**: Using modern config → No action needed
- ✅ **DSMR Component**: Not using DSMR → No action
- ✅ **DLMS Meter Component**: Not using DLMS → No action
- ✅ **Nextion Display**: Using LVGL instead → No action
- ✅ **Home Assistant Time Timezone**: Verify if using `time.homeassistant` with explicit timezone

**For Halo**: Zero breaking changes apply to your current configuration!

---

## 📋 IMPLEMENTATION ROADMAP

### Phase 1: Critical Upgrade (Immediate)
**Time**: 30 minutes

```bash
# 1. Update min_version in Halo-v1-Core.yaml
# 2. Test compilation and flashing
# 3. Verify all features work
```

**Files to Update**:
- `Halo-v1-Core.yaml`: Change `min_version: 2026.4.0` → `2026.6.0`

**Testing**:
- Compile firmware
- Flash to device
- Verify display rendering
- Verify BLE functionality
- Verify OTA still works

### Phase 2: Performance & Metadata (Within 1 week)
**Time**: 1-2 hours

```yaml
# 1. Add YAML frontmatter
# 2. Verify LVGL display metadata is auto-detected
# 3. Add esphome.build_flags if needed
# 4. Test config validation speed improvement
```

**Files to Update**:
- `Halo-v1.yaml`: Add frontmatter
- `Halo-v1-Core.yaml`: Add `esphome.build_flags` section if needed

### Phase 3: Optional Enhancements (Planned)
**Time**: As needed

- [ ] Add OTA partition table support if planning firmware conversion features
- [ ] Implement audio feedback if adding alerts
- [ ] Consider BMI270 IMU if want motion-awareness
- [ ] Evaluate new sensor options for future hardware versions

---

## 🎯 SPECIFIC FILE CHANGES

### 1. Update `Halo-v1-Core.yaml`

**Change**:
```yaml
# Before
  min_version: 2026.4.0

# After  
  min_version: 2026.6.0

# Add new section (optional but recommended)
esphome:
  name: halo-v1
  friendly_name: Halo Air Quality Sensor
  name_add_mac_suffix: true
  build_flags:  # NEW: Unified compiler flags
    # Add any specific compiler optimizations here
    # These now work for both PlatformIO and native IDF
```

### 2. Update `Halo-v1.yaml` (Add frontmatter)

**Add at the very top (before substitutions)**:
```yaml
---
author: "yashmulgaonkar"
version: "26.04.25.2142"
labels:
  - "air-quality-monitor"
  - "esphome"
  - "esp32-s3"
  - "lvgl"
description: "Air quality monitor with weather display on LilyGo T-Display-Long"
---

substitutions:
  # ... existing substitutions
```

### 3. Optional: Enhanced OTA in `Halo-v1.yaml`

**If you want partition updates capability**:
```yaml
ota:
  - platform: esphome
    id: ota_default
    allow_partition_access: true  # NEW: Enables partition table updates
    on_begin:
      - switch.turn_off: ble_scanner_switch
      - logger.log: "OTA started - BLE scanning paused"
    on_end:
      - switch.turn_on: ble_scanner_switch
      - logger.log: "OTA completed - BLE scanning resumed"
    on_error:
      - switch.turn_on: ble_scanner_switch
      - logger.log: "OTA failed - BLE scanning resumed"
```

---

## ✅ VERIFICATION CHECKLIST

After implementing changes:

- [ ] Update min_version to 2026.6.0
- [ ] Add YAML frontmatter (optional)
- [ ] Run `esphome compile halo-v1.yaml`
- [ ] Verify no new warnings/errors
- [ ] Flash firmware to device
- [ ] Test display rendering
- [ ] Test WiFi connectivity
- [ ] Test BLE provisioning
- [ ] Test OTA update
- [ ] Test weather display refresh
- [ ] Check sensor readings (CO2, PM, etc.)
- [ ] Verify air quality display
- [ ] Run `esphome config halo-v1.yaml` - should be noticeably faster

---

## 🔗 REFERENCES

### ESPHome 2026.5.0 Highlights
- [Changelog](https://esphome.io/changelog/2026.5.0/)
- Main Loop & Watchdog: 45% power reduction on OpenThread
- BLE Reliability Fix: Yale lock, Zigbee improvements
- OTA Enhancements: Partition tables, bootloader, soft-brick recovery
- Audio Modernization: New media source types, zero-copy buffers

### ESPHome 2026.6.0 Highlights  
- [Changelog](https://esphome.io/changelog/2026.6.0/)
- Device Builder 1.0.0 (replaces legacy dashboard)
- LVGL 4.5× faster validation
- Audio mixer: Any bit-depth support
- Display metadata: Auto byte-order selection
- BLE: Continuing reliability improvements
- WiFi/Ethernet: Memory optimizations when disabled at boot
- New IMU, audio DAC, USB-serial, pressure sensors

---

## 📊 IMPACT SUMMARY

| Feature | Priority | Effort | Impact | Recommended |
|---------|----------|--------|--------|------------|
| Min version upgrade | 🔴 Critical | 15 min | ⭐⭐⭐⭐⭐ | Yes |
| LVGL display metadata | 🟡 Medium | 5 min | ⭐⭐⭐ | Yes |
| BLE reliability fix | 🔴 Critical | 0 min | ⭐⭐⭐⭐ | Yes (automatic) |
| YAML frontmatter | 🟡 Medium | 10 min | ⭐⭐ | Yes |
| Build flags unification | 🟡 Medium | 15 min | ⭐⭐ | Future-proof |
| Enhanced OTA | 🟡 Medium | 5 min | ⭐⭐⭐ | Optional |
| Audio stack | 🟢 Low | N/A | ⭐ | If adding audio |
| New hardware | 🟢 Low | N/A | ⭐ | Future expansion |

---

**Next Steps**: Begin with Phase 1 (Critical Upgrade) - simply update the min_version and test thoroughly.
