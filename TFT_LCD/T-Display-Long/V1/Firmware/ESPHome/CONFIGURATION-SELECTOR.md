# Halo Configuration Selector

Use this guide to pick the right configuration for your needs.

## Quick Decision Tree

```
Do you want BLE?
│
├─ YES → Go to "BLE Compatible Configurations" below
│
└─ NO → Do you want weather?
    │
    ├─ YES → Go to "Weather Configurations" below
    │
    └─ NO → Go to "Base Configurations" below
```

## All 16 Configurations

### Base Configurations (No Weather, No BLE)

| Config | Features | Pages | RAM Usage | Free RAM | LVGL Buffer |
|--------|----------|-------|-----------|----------|-------------|
| **1** | Clock only | 1 | ~60KB | ~150KB | 35% |
| **2** | Clock + AirQ | 2 | ~80KB | ~130KB | 30% |
| **3** | Clock + WiFi | 2 | ~85KB | ~125KB | 30% |
| **4** | Clock + WiFi + WireGuard | 2 | ~100KB | ~110KB | 28% |
| **5** | Clock + AirQ + WiFi | 3 | ~105KB | ~105KB | 28% |
| **6** | Clock + AirQ + WiFi + WireGuard | 3 | ~120KB | ~90KB | 25% |

**Pages Packages:**
- Config 1: `lvgl-pages-clock-only.yaml`
- Config 2: `lvgl-pages-airq-only.yaml`
- Config 3: Need to create (clock + WiFi)
- Config 4: Need to create (clock + WiFi + WireGuard)
- Config 5: `lvgl-pages-airq-wifi.yaml`
- Config 6: Need to create (AirQ + WiFi + WireGuard, no weather)

---

### Weather Configurations (No BLE)

| Config | Features | Sensors | Pages | RAM Usage | Free RAM |
|--------|----------|---------|-------|-----------|----------|
| **7** | Clock + Weather Forecast | Current (8) | 1 | ~110KB | ~100KB |
| **8** | Clock + Forecast + Daily | Current + Daily (53) | 6 | ~165KB | ~45KB |
| **9** | Clock + Forecast + Hourly | Current + Hourly (80) | 3 | ~155KB | ~55KB |
| **10** | Clock + Daily only | Daily (45) | 5 | ~160KB | ~50KB |
| **11** | Clock + Hourly only | Hourly (72) | 2 | ~150KB | ~60KB |
| **12** | Clock + Full Weather | All (125) | 8 | ~185KB | ~25KB |

**Sensor Packages:**
- Configs 7, 8, 9, 12: `weather-sensors-current.yaml`
- Configs 8, 10, 12: `weather-sensors-daily.yaml`
- Configs 9, 11, 12: `weather-sensors-hourly.yaml`

**Pages Packages:**
- Config 7: Need to create (forecast only)
- Config 8: Need to create (forecast + daily)
- Config 9: Need to create (forecast + hourly)
- Config 10: Need to create (daily only, no forecast)
- Config 11: Need to create (hourly only)
- Config 12: `lvgl-pages-weather.yaml`

---

### Weather + AirQ Configurations

| Config | Features | Sensors | RAM Usage | Free RAM | Status |
|--------|----------|---------|-----------|----------|--------|
| **13a** | Config 7 + AirQ | Current (8) | ~130KB | ~80KB | ✅ Good |
| **13b** | Config 8 + AirQ | Current + Daily (53) | ~185KB | ~25KB | ⚠️ Tight |
| **13c** | Config 9 + AirQ | Current + Hourly (80) | ~175KB | ~35KB | ⚠️ Tight |
| **13d** | Config 10 + AirQ | Daily (45) | ~180KB | ~30KB | ⚠️ Tight |
| **13e** | Config 11 + AirQ | Hourly (72) | ~170KB | ~40KB | ⚠️ Tight |
| **13f** | Config 12 + AirQ | All (125) | ~205KB | ~5KB | ❌ Too tight |

**Pages Packages:**
- Config 13f: `lvgl-pages-airq-weather.yaml`
- Others: Need to create custom combinations

---

### Weather + WiFi Configurations

| Config | Features | Sensors | RAM Usage | Free RAM | Status |
|--------|----------|---------|-----------|----------|--------|
| **14a** | Config 7 + WiFi | Current (8) | ~135KB | ~75KB | ✅ Good |
| **14b** | Config 8 + WiFi | Current + Daily (53) | ~190KB | ~20KB | ⚠️ Tight |
| **14c** | Config 9 + WiFi | Current + Hourly (80) | ~180KB | ~30KB | ⚠️ Tight |
| **14d** | Config 10 + WiFi | Daily (45) | ~185KB | ~25KB | ⚠️ Tight |
| **14e** | Config 11 + WiFi | Hourly (72) | ~175KB | ~35KB | ⚠️ Tight |
| **14f** | Config 12 + WiFi | All (125) | ~210KB | ~0KB | ❌ Won't work |

**Pages Packages:** Need to create custom combinations

---

### Weather + WiFi + WireGuard Configurations

| Config | Features | Sensors | RAM Usage | Free RAM | Status |
|--------|----------|---------|-----------|----------|--------|
| **15a** | Config 7 + WiFi + WG | Current (8) | ~150KB | ~60KB | ✅ Good |
| **15b** | Config 8 + WiFi + WG | Current + Daily (53) | ~205KB | ~5KB | ❌ Too tight |
| **15c** | Config 9 + WiFi + WG | Current + Hourly (80) | ~195KB | ~15KB | ❌ Too tight |
| **15d** | Config 10 + WiFi + WG | Daily (45) | ~200KB | ~10KB | ❌ Too tight |
| **15e** | Config 11 + WiFi + WG | Hourly (72) | ~190KB | ~20KB | ⚠️ Tight |
| **15f** | Config 12 + WiFi + WG | All (125) | ~225KB | -15KB | ❌ Won't compile |

**Pages Packages:** Need to create custom combinations

---

### Everything Configurations

| Config | Features | Sensors | RAM Usage | Free RAM | Status |
|--------|----------|---------|-----------|----------|--------|
| **16a** | AirQ + WiFi + Forecast | Current (8) | ~140KB | ~70KB | ✅ Good |
| **16b** | AirQ + WiFi + Forecast + Daily | Current + Daily (53) | ~195KB | ~15KB | ❌ Too tight |
| **16c** | AirQ + WiFi + Forecast + Hourly | Current + Hourly (80) | ~185KB | ~25KB | ⚠️ Tight |
| **16d** | AirQ + WiFi + WG + Forecast | Current (8) | ~155KB | ~55KB | ✅ Good |
| **16e** | AirQ + WiFi + WG + Full Weather | All (125) | ~230KB | -20KB | ❌ Won't compile |

**Pages Packages:**
- Config 16e: `lvgl-pages-full-wireguard.yaml` (but won't work - too much memory)
- Others: Need to create custom

---

## BLE Compatible Configurations

**BLE adds ~60KB RAM usage**

| Config | Features + BLE | Total RAM | Free RAM | Status |
|--------|----------------|-----------|----------|--------|
| **1 + BLE** | Clock only | ~120KB | ~90KB | ✅ Great |
| **2 + BLE** | Clock + AirQ | ~140KB | ~70KB | ✅ Good |
| **3 + BLE** | Clock + WiFi | ~145KB | ~65KB | ✅ Good |
| **4 + BLE** | Clock + WiFi + WG | ~160KB | ~50KB | ✅ OK |
| **5 + BLE** | Clock + AirQ + WiFi | ~165KB | ~45KB | ⚠️ Tight |
| **6 + BLE** | Clock + AirQ + WiFi + WG | ~180KB | ~30KB | ⚠️ Very tight |
| **7 + BLE** | Clock + Weather Forecast | ~170KB | ~40KB | ⚠️ Very tight |
| **8+ + BLE** | Any with Daily/Hourly | >210KB | <0KB | ❌ OOM |

**LVGL Buffer with BLE:** 15% (required)

**Recommendations:**
- ✅ **Best:** Config 2 + BLE (Clock + AirQ + BLE)
- ✅ **Good:** Config 7 + BLE (Clock + Minimal Weather + BLE)
- ⚠️ **Risky:** Config 5 or 6 + BLE (monitor Free Heap!)
- ❌ **Don't:** Config 8+ + BLE (will crash)

---

## Recommended Configurations

### For Most Users
**Config 8: Clock + Forecast + Daily (NO BLE)**
- All weather pages (current + 9-day forecast)
- AirQ monitoring
- WiFi monitoring optional
- ~45KB free RAM (comfortable)

### For BLE Users
**Config 2 + BLE: Clock + AirQ + BLE**
- BLE bluetooth proxy
- Air quality monitoring
- ~70KB free RAM (safe)
- No weather (add Config 7 if you must have minimal weather)

### For Weather Enthusiasts
**Config 12: Full Weather (NO BLE, NO AirQ)**
- All 8 weather pages
- Current + Daily + Hourly forecasts
- ~25KB free RAM (works but tight)

### For Minimalists
**Config 1: Clock Only**
- Just the clock
- Maximum performance
- ~150KB free RAM
- Can add BLE easily

---

## How to Choose

### Step 1: Do you need BLE?
- **YES** → Pick from Configs 1-7 + BLE only
- **NO** → Continue to Step 2

### Step 2: Do you need weather?
- **NO** → Pick from Configs 1-6 (base)
- **YES** → Continue to Step 3

### Step 3: Which weather do you need?
- **Just current forecast** → Config 7 (or 7 + BLE)
- **Current + multi-day** → Config 8
- **Current + hourly** → Config 9
- **Everything** → Config 12 (tight on RAM)

### Step 4: Add extras
- **Need AirQ?** → Add 20KB RAM usage
- **Need WiFi monitoring?** → Add 5KB RAM usage
- **Need WireGuard?** → Add 15KB RAM usage

### Step 5: Check total RAM
- Total RAM < 150KB → ✅ Safe
- Total RAM 150-180KB → ⚠️ Test thoroughly
- Total RAM 180-200KB → ⚠️ Very tight, may crash
- Total RAM > 200KB → ❌ Won't work

---

## Pages Packages Available

### Pre-built (Exist Now):
- `lvgl-pages-clock-only.yaml` - Config 1
- `lvgl-pages-airq-only.yaml` - Config 2
- `lvgl-pages-airq-wifi.yaml` - Config 5
- `lvgl-pages-weather.yaml` - Config 12
- `lvgl-pages-airq-weather.yaml` - Config 13f
- `lvgl-pages-full.yaml` - Config 16b (no WG)
- `lvgl-pages-full-wireguard.yaml` - Config 16e

### Need to Create:
- Config 3: Clock + WiFi
- Config 4: Clock + WiFi + WireGuard
- Config 6: Clock + AirQ + WiFi + WireGuard (no weather)
- Config 7: Clock + Forecast only
- Config 8: Clock + Forecast + Daily
- Config 9: Clock + Forecast + Hourly
- Config 10: Clock + Daily only
- Config 11: Clock + Hourly only
- All variations of 13-15

---

## Memory Formula

```
Base System: ~60KB
LVGL Buffer:
  - 15% = ~35KB (with BLE)
  - 25% = ~58KB (without BLE)
  - 30% = ~70KB (minimal features)
  - 35% = ~82KB (clock only)

AirQ: ~20KB
WiFi Monitoring: ~5KB
WireGuard: ~15KB
BLE: ~60KB

Weather Core + Fonts: ~25KB
Weather Sensors Current: ~5KB
Weather Sensors Daily: ~30KB
Weather Sensors Hourly: ~45KB

Total Available: ~210KB internal RAM
Minimum Free: 60KB (for stability)
```

---

## Quick Reference Table

| Want | Use Config | BLE OK? | Free RAM |
|------|------------|---------|----------|
| Just clock | 1 | ✅ Yes | ~150KB |
| Clock + AirQ | 2 | ✅ Yes | ~130KB |
| Clock + AirQ + BLE | 2 + BLE | ✅ | ~70KB |
| Weather forecast only | 7 | ✅ Yes (tight) | ~100KB |
| Weather + BLE | 7 + BLE | ⚠️ | ~40KB |
| Full 10-day forecast | 8 | ❌ No BLE | ~45KB |
| Hourly forecasts | 9 | ❌ No BLE | ~55KB |
| Everything weather | 12 | ❌ No BLE | ~25KB |
| Weather + AirQ | 13a-f | ❌ No BLE | Varies |
| Absolute maximum | 16a | ❌ No BLE | ~70KB |

---

## Status Legend

- ✅ **Great:** >80KB free RAM - Very stable
- ✅ **Good:** 60-80KB free RAM - Stable
- ✅ **OK:** 50-60KB free RAM - Should be fine
- ⚠️ **Tight:** 30-50KB free RAM - Test thoroughly, monitor Free Heap
- ⚠️ **Very Tight:** 15-30KB free RAM - May crash under load
- ❌ **Too Tight:** 5-15KB free RAM - Will likely crash
- ❌ **Won't Work:** <5KB or negative - Won't compile/run
- ❌ **OOM:** Out of Memory - Definitely won't work

---

Use [halo-v1-template-modular.yaml](halo-v1-template-modular.yaml) as your starting point and uncomment the packages for your chosen configuration!
