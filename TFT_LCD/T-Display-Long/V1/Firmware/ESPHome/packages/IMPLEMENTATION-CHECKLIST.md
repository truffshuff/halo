# PSRAM Optimization Implementation Checklist

Use this checklist to track your implementation progress.

---

## ✅ Implementation Phase (COMPLETE)

- [x] Convert all 125 weather sensors to use PSRAM buffers
- [x] Remove all `char buffer[]` stack allocations
- [x] Add memory monitoring sensors
- [x] Update header documentation in weather-sensors.yaml
- [x] Create comprehensive documentation
- [x] Create verification script
- [x] Verify zero char buffer[] remaining

---

## 📋 Deployment Checklist (YOUR TASKS)

### Pre-Deployment

- [ ] Read QUICK-START-PSRAM.md
- [ ] Run verification: `python3 verify-psram-optimization.py`
- [ ] Confirm all checks pass ✅

### Configuration Update

- [ ] Open your main device config file (e.g., `halo-v1-79e384.yaml`)
- [ ] Ensure packages section includes:
  ```yaml
  packages:
    psram_helpers: !include packages/psram-helpers.yaml  # FIRST!
    weather_sensors: !include packages/weather-sensors.yaml
  ```
- [ ] Verify psram_helpers comes BEFORE weather_sensors
- [ ] Save the file

### Compilation

- [ ] Run: `esphome compile your-device-config.yaml`
- [ ] Check for compilation errors
- [ ] Verify no memory warnings in compile output
- [ ] Note the binary size (should be similar to before)

### Upload & Boot

- [ ] Upload firmware: `esphome upload your-device-config.yaml`
- [ ] Monitor logs: `esphome logs your-device-config.yaml`
- [ ] Look for boot message: "PSRAM buffers allocated successfully"
- [ ] Verify buffer addresses start with `0x3c...` (PSRAM region)
- [ ] Check "Free internal heap" value (should be 120-150KB)

### Functional Testing

- [ ] Verify all weather sensors display data correctly
- [ ] Check temperature values are shown
- [ ] Check forecast data appears
- [ ] Verify precipitation probabilities display
- [ ] Confirm no "--" fallback values (unless data unavailable)

### Stability Testing

- [ ] Enable BLE (if disabled)
- [ ] Verify WiFi and BLE work simultaneously
- [ ] Check for memory errors in logs
- [ ] Let device run for 1 hour - check for crashes
- [ ] Let device run overnight - verify stability

### Monitoring Setup

- [ ] Add "Free Internal Heap" sensor to Home Assistant dashboard
- [ ] Add "Free PSRAM" sensor to dashboard
- [ ] Add "Largest Internal Block" sensor to dashboard
- [ ] Set up alert if heap drops below 100KB (optional)
- [ ] Monitor for 24 hours

---

## 📊 Success Verification

Mark these items once confirmed:

### Boot Phase
- [ ] Log shows: "Allocating weather sensor buffers in PSRAM..."
- [ ] Log shows: "✓ PSRAM buffers allocated successfully"
- [ ] Log shows: "Format buffer: 256 bytes @ 0x3c..."
- [ ] Log shows: "Temp buffer: 64 bytes @ 0x3c..."
- [ ] Log shows: "Condition buffer: 128 bytes @ 0x3c..."
- [ ] Free internal heap reported as >120KB

### Runtime Phase
- [ ] All weather sensors updating normally
- [ ] Temperature displays show correct format ("72°F")
- [ ] Forecast displays show "H:75° L:62°" format
- [ ] Precipitation shows "Chance: 60%" format
- [ ] No "ERROR" or "WARNING" about memory in logs

### Memory Metrics
- [ ] Free Internal Heap: 120-150KB ✅ (was 50-80KB)
- [ ] Free PSRAM: >7MB ✅
- [ ] Largest Internal Block: >80KB ✅
- [ ] No heap fragmentation warnings ✅

### Stability Metrics  
- [ ] Device runs >24 hours without restart ✅
- [ ] BLE + WiFi work together ✅
- [ ] No crashes or reboots ✅
- [ ] Weather updates continue working ✅

---

## 🐛 Troubleshooting Checklist

If something doesn't work, check these:

### Sensors Show "--" Values
- [ ] Check package order (psram_helpers before weather_sensors)
- [ ] Verify boot log shows "PSRAM buffers allocated"
- [ ] Check Home Assistant entities are correct
- [ ] Verify device has network connectivity

### Low Memory Warnings
- [ ] Confirm PSRAM is enabled in board config
- [ ] Check for memory leaks in other components
- [ ] Verify buffer initialization succeeded
- [ ] Look for other large heap allocations

### Compilation Errors
- [ ] Verify all package files exist
- [ ] Check YAML indentation (use spaces, not tabs)
- [ ] Ensure psram_allocator.h exists
- [ ] Check ESPHome version (need recent version)

### Runtime Crashes
- [ ] Check logs for stack traces
- [ ] Verify PSRAM configuration in core config
- [ ] Look for null pointer issues
- [ ] Check for uninitialized buffer access

---

## 📈 Performance Baseline (Record Your Results)

### Before Optimization
- Free Internal Heap: _________ KB
- Free PSRAM: _________ MB
- Uptime before crash: _________ hours
- BLE working: ☐ Yes ☐ No
- WiFi stable: ☐ Yes ☐ No

### After Optimization
- Free Internal Heap: _________ KB (Target: 120-150KB)
- Free PSRAM: _________ MB
- Uptime: _________ hours (Target: >24 hours)
- BLE working: ☐ Yes ☐ No (Target: Yes)
- WiFi stable: ☐ Yes ☐ No (Target: Yes)

### Improvement
- Internal Heap Gained: _________ KB (Expected: ~70KB)
- Stability Improved: ☐ Yes ☐ No ☐ Same
- Features Working: ☐ More ☐ Same ☐ Less

---

## 🎯 Sign-Off

Once all items above are checked and verified:

- [ ] All sensors converted and working ✅
- [ ] Memory improvements confirmed ✅
- [ ] Stability improved ✅
- [ ] Documentation reviewed ✅
- [ ] Monitoring set up ✅

**Implementation Date:** _________________

**Verified By:** _________________

**Status:** ☐ Production Ready ☐ Needs More Testing

---

## 📝 Notes

Add any observations or issues encountered:

```
_______________________________________________________________________

_______________________________________________________________________

_______________________________________________________________________

_______________________________________________________________________
```

---

## 📚 Reference

- Quick Start: `QUICK-START-PSRAM.md`
- Full Guide: `README-PSRAM-OPTIMIZATION.md`
- Technical Details: `PSRAM-OPTIMIZATION-SUMMARY.md`
- Verification: `python3 verify-psram-optimization.py`
