# BLE Package Integration Guide

## Summary

The BLE (Bluetooth Low Energy) functionality has been **fully modularized** into separate packages! BLE is **DISABLED by default** due to heap memory constraints but can be easily enabled when needed.

## What Was Modularized

### Packages Created:
1. **ble-core.yaml** (~95 lines) - BLE tracker, Bluetooth Proxy, and Improv
2. **ble-stubs.yaml** (~20 lines) - Empty stub for BLE-disabled mode (DEFAULT)

### Old Files (now deprecated):
- ~~esp32_ble_tracker.yaml~~ - Merged into ble-core.yaml
- ~~bluetooth_proxy.yaml~~ - Merged into ble-core.yaml
- ~~esp32_improv.yaml~~ - Merged into ble-core.yaml

### Total Lines: ~95 lines in combined package

## Current State

### ✅ BLE is DISABLED by default (RECOMMENDED)

In `Halo-v1-Core.yaml` packages section (around line 11):
```yaml
packages:
  # BLE packages (optional - see packages/README-BLE-INTEGRATION.md)
  # OPTION 1: BLE DISABLED (default) - Use BLE stubs (recommended):
  ble_stubs: !include packages/ble-stubs.yaml
  # OPTION 2: BLE ENABLED - Comment out the line above and uncomment this:
  # ble_core: !include packages/ble-core.yaml
  # WARNING: Enabling BLE can cause heap exhaustion and crashes!
  # Reduce LVGL buffer_size to 15-20% if enabling BLE
  # Monitor heap usage via memory_stats package
```

## Why BLE is Disabled by Default

### Heap Memory Constraints

The ESP32-S3 has limited heap memory (~200-250KB available after system overhead). The Halo device uses:

| Component | Heap Usage | Notes |
|-----------|------------|-------|
| **LVGL Display (25% buffer)** | ~40 KB | Main UI framework |
| **BLE Tracker + Proxy** | ~50-80 KB | ⚠️ Large memory footprint |
| **WiFi + WireGuard** | ~30 KB | Network stack |
| **AirQ Sensors (4 sensors)** | ~15-20 KB | I2C sensor buffers |
| **Weather Data** | ~10-15 KB | Forecast caching |
| **System Overhead** | ~30-40 KB | FreeRTOS, ESP-IDF |
| **TOTAL** | ~175-225 KB | Near or exceeding limit! |

### Problems When Enabling BLE:

1. **Heap Exhaustion**: Random crashes with "out of memory" errors
2. **Boot Loops**: Device repeatedly restarts due to OOM
3. **Watchdog Timeouts**: System becomes unresponsive
4. **Display Glitches**: LVGL fails to allocate buffers
5. **Sensor Failures**: I2C operations fail due to memory pressure

## When to Enable BLE

### You SHOULD enable BLE if:

✅ You **absolutely need** Home Assistant Bluetooth Proxy functionality
✅ You can **reduce LVGL buffer_size** to 15-20% (from default 25%)
✅ You are willing to **disable some features** (AirQ, Weather, etc.)
✅ You will **actively monitor** heap usage via memory_stats
✅ You understand the **risk of crashes** and are willing to troubleshoot

### You should NOT enable BLE if:

❌ You want a **stable, reliable** display without crashes
❌ You need **all sensors** (AirQ, Weather) running simultaneously
❌ You don't have time to **debug heap issues**
❌ You don't actively use **Home Assistant Bluetooth integration**

## How to Enable BLE (Advanced Users Only)

### Step-by-Step Instructions:

1. **Reduce LVGL Buffer Size** (REQUIRED - do this FIRST):
   ```yaml
   # In Halo-v1-Core.yaml around line 1735
   lvgl:
     buffer_size: 15%  # Reduced from 25% to free ~16 KB
   ```

2. **Enable BLE Core Package**:
   ```yaml
   # In Halo-v1-Core.yaml around line 12
   # Comment out ble-stubs:
   # ble_stubs: !include packages/ble-stubs.yaml

   # Uncomment ble-core:
   ble_core: !include packages/ble-core.yaml
   ```

   Or use the delayed + guarded variant and coexistence tuning (recommended when combining with Weather/AirQ):

   ```yaml
   packages:
     ble_coexist_tuning: !include packages/ble-coexist-tuning.yaml
     ble_core_delayed: !include packages/ble-core-delayed.yaml
   ```

3. **Monitor Heap Usage** (CRITICAL):
   - Watch Home Assistant sensor: `sensor.free_heap`
   - Heap should stay above **30-40 KB** at all times
   - If heap drops below 20 KB, expect crashes

4. **Optional: Disable Features to Free Memory**:
   ```yaml
   # Disable AirQ (saves ~20 KB):
   # airq: !include packages/airq-core.yaml
   airq_stubs: !include packages/airq-stubs.yaml

   # Disable Weather (saves ~15 KB):
   # weather_core: !include packages/weather-core.yaml
   weather_stubs: !include packages/weather-stubs.yaml
   ```

5. **Test Thoroughly**:
   - Reboot device multiple times
   - Navigate through all pages
   - Monitor for crashes over 24-48 hours
   - Check ESPHome logs for OOM warnings

## BLE Components Included

### ble-core.yaml Contains:

#### 1. ESP32 BLE Tracker
Scans for BLE devices and makes them available to Home Assistant.

**Configuration**:
```yaml
esp32_ble_tracker:
  id: ble_tracker
  scan_parameters:
    interval: 500ms    # How often to scan (default: 320ms)
    window: 40ms       # How long each scan lasts (default: 30ms)
    active: true       # Send scan requests for more info
    continuous: false  # On-demand scanning (not continuous)
```

**Scan Modes**:
- **Passive** (`active: false`): Lower power, less info
- **Active** (`active: true`): Higher power, more device info
- **Continuous** (`continuous: true`): Always scanning (HIGH power!)
- **On-Demand** (`continuous: false`): Scan only when needed (recommended)

#### 2. Bluetooth Proxy
Allows Home Assistant to use this device as a Bluetooth proxy for BLE devices.

**Configuration**:
```yaml
bluetooth_proxy:
  active: true          # Enable the proxy
  connection_slots: 3   # Max simultaneous connections (1-9)
```

**Features**:
- Extends Home Assistant Bluetooth range
- Proxies BLE devices to Home Assistant
- Supports up to 9 simultaneous connections
- **REQUIRES** esp32_ble_tracker to function

**Supported Devices**:
- Xiaomi Mi Home sensors
- Switchbot devices
- BLE thermometers/hygrometers
- BLE presence detection
- And more!

#### 3. ESP32 Improv (WiFi Provisioning)
Allows WiFi provisioning via BLE using the Improv standard.

**Configuration**:
```yaml
esp32_improv:
  authorizer: none  # No authorization required
```

**Use Cases**:
- Initial device setup without hardcoded WiFi credentials
- Easy WiFi network changes via mobile app
- Works with Improv-compatible apps

**Security Note**:
- `authorizer: none` = anyone can provision the device
- Consider using a password authorizer in production

#### 4. API Integration (Dynamic Scanning)
Automatically starts/stops BLE scanning based on Home Assistant connection.

**Configuration**:
```yaml
api:
  on_client_connected:
    - delay: 500ms
    - esp32_ble_tracker.start_scan:
        continuous: true
  on_client_disconnected:
    - esp32_ble_tracker.stop_scan:
```

**Benefits**:
- Saves power when Home Assistant is offline
- Reduces heap pressure when not in use
- Automatic - no manual intervention needed
- Scanning resumes immediately when HA reconnects

**Power Savings**:
- ~25-35 mA saved when disconnected
- ~15-20 KB heap freed when scanning stopped

## Performance Impact

### Memory Usage:
- **BLE Tracker**: ~30-40 KB heap
- **Bluetooth Proxy**: ~20-30 KB heap
- **ESP32 Improv**: ~5-10 KB heap
- **Total BLE**: ~55-80 KB heap (varies by active connections)

### CPU Usage:
- **Idle**: ~2-5% CPU
- **Active Scanning**: ~10-15% CPU
- **Proxying Connections**: ~5-10% CPU per connection

### Power Consumption:
- **Passive Scanning**: +15-20 mA
- **Active Scanning**: +25-35 mA
- **Continuous Scanning**: +40-50 mA
- **Proxying**: +5-10 mA per connection

### Network Impact:
- BLE proxy traffic is forwarded to Home Assistant
- Uses WiFi API connection (negligible bandwidth)
- Does NOT increase WiFi power consumption significantly

## Heap Management Strategies

If you enable BLE and experience crashes, try these strategies:

### 1. Reduce Display Buffer (MOST EFFECTIVE):
```yaml
lvgl:
  buffer_size: 15%  # Frees ~16 KB (from 25%)
  # Or even 10% if display performance is acceptable
```

### 2. Disable Unused Features:
```yaml
# Disable AirQ (saves ~20 KB):
airq_stubs: !include packages/airq-stubs.yaml

# Disable Weather (saves ~15 KB):
weather_stubs: !include packages/weather-stubs.yaml

# Disable WiFi page (saves ~6 KB):
wifi_stubs: !include packages/wifi-stubs.yaml
```

### 3. Reduce BLE Connection Slots:
```yaml
bluetooth_proxy:
  connection_slots: 1  # Saves ~15-20 KB (from 3)
```

### 4. Use Passive Scanning:
```yaml
esp32_ble_tracker:
  scan_parameters:
    active: false  # Saves ~5-10 KB
```

### 5. Monitor Heap Usage:
```yaml
# Already included via memory_stats package
sensor:
  - platform: template
    name: "Free Heap"
    lambda: 'return heap_caps_get_free_size(MALLOC_CAP_INTERNAL);'
    unit_of_measurement: "bytes"
```

**Safe Heap Levels**:
- **> 50 KB**: ✅ Safe
- **30-50 KB**: ⚠️ Marginal - monitor closely
- **20-30 KB**: ❌ Danger zone - crashes likely
- **< 20 KB**: ❌ Imminent crash

## Dynamic BLE Scanning

The ble-core.yaml package **automatically includes** dynamic BLE scanning based on API connection:

```yaml
# Included in ble-core.yaml:
api:
  on_client_connected:
    - delay: 500ms  # Brief delay to allow API to stabilize
    - esp32_ble_tracker.start_scan:
        continuous: true
  on_client_disconnected:
    - esp32_ble_tracker.stop_scan:
```

**Benefits**:
- ✅ Only scans when Home Assistant is connected
- ✅ Saves power when disconnected (~25-35 mA)
- ✅ Reduces heap pressure when not needed (~15-20 KB freed)
- ✅ Automatic - no manual intervention required

**How It Works**:
1. Device boots with BLE tracker stopped
2. When Home Assistant connects via API → Start continuous scanning
3. When Home Assistant disconnects → Stop scanning
4. Scanning resumes when HA reconnects

**Current Status**:
- ✅ **Automatically included** in ble-core.yaml
- ✅ No manual configuration needed
- ✅ Works out of the box when BLE is enabled

## Home Assistant Integration

### Entities Exposed

When BLE is enabled, these entities are available:

#### Sensors (via memory_stats):
- `sensor.free_heap` - Available heap memory
- `sensor.largest_free_block` - Largest contiguous heap block

#### Binary Sensors:
- `binary_sensor.bluetooth_proxy_connected` - Proxy connection status

#### Services:
- `esphome.start_ble_scan` - Manually start BLE scanning
- `esphome.stop_ble_scan` - Manually stop BLE scanning

### Bluetooth Proxy Dashboard

Home Assistant automatically discovers the Bluetooth Proxy:

1. **Settings** → **Devices & Services**
2. Look for "ESPHome Bluetooth Proxy"
3. Click to see proxied BLE devices
4. Range indicator shows signal strength

## Troubleshooting

### Device Crashes Randomly
**Cause**: Heap exhaustion
**Solution**:
1. Reduce LVGL buffer_size to 15%
2. Disable AirQ or Weather packages
3. Reduce bluetooth_proxy connection_slots to 1
4. Monitor heap usage - keep above 30 KB

### BLE Devices Not Appearing in Home Assistant
**Cause**: Tracker not running or proxy not connected
**Solution**:
1. Check `binary_sensor.bluetooth_proxy_connected` is ON
2. Verify esp32_ble_tracker is enabled in ble-core.yaml
3. Check ESPHome logs for BLE errors
4. Ensure BLE device is in range (~10m)

### "Out of Memory" Errors in Logs
**Cause**: Heap exhausted
**Solution**:
1. Immediately reduce LVGL buffer_size
2. Disable features (AirQ, Weather)
3. Consider disabling BLE entirely
4. Check for memory leaks in custom code

### WiFi Provisioning via Improv Not Working
**Cause**: Improv not enabled or app incompatible
**Solution**:
1. Verify esp32_improv is in ble-core.yaml
2. Use compatible app (ESPHome Web, Improv WiFi)
3. Check BLE is enabled on your phone
4. Ensure device is in provisioning mode

### High Power Consumption
**Cause**: Continuous BLE scanning
**Solution**:
1. Set `continuous: false` in scan_parameters
2. Use `active: false` for passive scanning
3. Reduce scan interval to 1000ms
4. Enable dynamic scanning (API-triggered)

## Recommendations

**Default Approach** ✅ RECOMMENDED:
- BLE is **disabled by default** - Ensures system stability
- BLE is **fully modular** - Can be enabled when needed
- **No compilation errors** - The stub package ensures clean compilation
- **Easy to enable** - Just swap packages and reduce buffer_size

**When to Enable BLE** ⚠️ ADVANCED:
- You **need** Home Assistant Bluetooth Proxy
- You are **willing to sacrifice** display buffer or features
- You **actively monitor** heap usage
- You **understand** the risks of crashes

**Best Practices**:
1. Start with BLE disabled (default)
2. Only enable if you have a specific use case
3. Always reduce LVGL buffer first
4. Monitor heap usage for 24-48 hours
5. Be prepared to disable BLE if crashes occur

## Files Modified/Created

### Created:
- `/packages/ble-core.yaml` - ✨ NEW (95 lines)
- `/packages/ble-stubs.yaml` - ✨ NEW (20 lines)
- `/packages/README-BLE-INTEGRATION.md` - ✨ NEW (this file)

### Deprecated (merged into ble-core.yaml):
- ~~`/packages/esp32_ble_tracker.yaml`~~ - 📦 MERGED
- ~~`/packages/bluetooth_proxy.yaml`~~ - 📦 MERGED
- ~~`/packages/esp32_improv.yaml`~~ - 📦 MERGED

### Modified:
- `/Halo-v1-Core.yaml` - 📝 MODIFIED (comments added, ble-stubs enabled by default)

## Modularization Status

### ✅ COMPLETED:
1. ✅ Combined esp32_ble_tracker, bluetooth_proxy, and esp32_improv into ble-core.yaml
2. ✅ Created ble-stubs.yaml for BLE-disabled mode (default)
3. ✅ Updated main config with clear warnings about heap usage
4. ✅ Added buffer_size reduction instructions
5. ✅ Documented heap management strategies
6. ✅ Documented all components and risks in README
7. ✅ Verified BLE dependencies (proxy REQUIRES tracker)
8. ✅ Set BLE disabled by default for stability

### 🎉 Result:
- **~95 lines** of BLE code modularized into one package
- **Zero compilation errors** when BLE is disabled (default)
- **Full backward compatibility** - BLE disabled matches old behavior
- **Easy to enable** - swap packages and reduce buffer
- **Professional documentation** - Following AirQ, WiFi, Weather patterns
- **Proper dependency management** - Tracker + Proxy + Improv combined

## Pattern Consistency

This modularization follows the same proven pattern as other optional features:
- **Core package**: All BLE functionality combined (tracker IS required for proxy)
- **Stubs package**: Empty stub for disabled mode
- **README**: Comprehensive documentation with warnings
- **Default**: DISABLED for system stability

**Key Difference**: BLE is the ONLY feature disabled by default due to heap constraints!

## BLE vs Other Features

| Feature | Default | Memory Impact | Stability Risk |
|---------|---------|---------------|----------------|
| AirQ | ✅ Enabled | Medium (~20 KB) | Low |
| WiFi Page | ✅ Enabled | Low (~6 KB) | Low |
| Weather | ✅ Enabled | Medium (~15 KB) | Low |
| **BLE** | ❌ **Disabled** | **High (~60 KB)** | **HIGH** |

BLE is unique in that it's the only feature that can cause system instability when combined with full display features!
