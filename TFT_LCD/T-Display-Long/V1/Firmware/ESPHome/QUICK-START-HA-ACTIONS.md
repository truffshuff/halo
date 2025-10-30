# Quick Start: Weather via Home Assistant Actions

## 🚀 5-Minute Setup for BLE + Weather

This quick start gets you up and running with the new HA Actions approach in minutes.

---

## Step 1: Copy the Template (30 seconds)

```bash
cd TFT_LCD/T-Display-Long/V1/Firmware/ESPHome
cp halo-v1-template-dynamic.yaml halo-v1-yourname.yaml
```

---

## Step 2: Configure Features (2 minutes)

Edit `halo-v1-yourname.yaml` and set your features:

```yaml
substitutions:
  name: "halo-v1-yourname"           # CHANGE THIS!
  friendly_name: "Halo V1"

  # Enable what you want
  feature_ble: "true"                # BLE presence
  feature_weather: "true"            # Weather via HA actions
  weather_entity_id: "weather.home"  # YOUR weather entity!

  # Optional features (set to "true" to enable)
  feature_airq: "false"
  feature_wifi_page: "false"
```

---

## Step 3: Uncomment Packages (1 minute)

Find and uncomment these sections in the same file:

### For BLE:
```yaml
# UNCOMMENT THIS:
ble_core:
  url: https://github.com/truffshuff/halo.git/
  ref: main
  files:
    - TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/ble-core-simplified.yaml
  refresh: always
```

### For Weather:
```yaml
# UNCOMMENT ALL THREE:
weather_ha_actions:
  url: https://github.com/truffshuff/halo.git/
  ref: main
  files:
    - TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/weather-ha-actions.yaml
  refresh: always

weather_core:
  url: https://github.com/truffshuff/halo.git/
  ref: main
  files:
    - TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/weather-core.yaml
  refresh: always

weather_fonts:
  url: https://github.com/truffshuff/halo.git/
  ref: main
  files:
    - TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/weather-fonts-text.yaml
  refresh: always
```

### Choose LVGL Pages:
```yaml
# CHANGE FROM clock-only to weather-forecast-only:
lvgl_pages:
  url: https://github.com/truffshuff/halo.git/
  ref: main
  files: [TFT_LCD/T-Display-Long/V1/Firmware/ESPHome/packages/lvgl-pages-weather-forecast-only.yaml]
  refresh: always
```

---

## Step 4: Compile & Flash (1 minute)

```bash
esphome run halo-v1-yourname.yaml
```

---

## Step 5: Verify It Works (30 seconds)

Check the logs for:
```
[weather] Fetching weather data from Home Assistant...
[weather] ✓ Weather data fetch complete!
[weather] Current: 72.0°F, sunny
```

---

## 🎉 Done!

You now have:
- ✅ BLE presence detection working
- ✅ Full weather data (10-day + hourly forecasts)
- ✅ ~70KB more free RAM than before
- ✅ Auto-refresh every 5 minutes while viewing weather

---

## Common Issues

### "Weather entity not found"
**Fix:** Change `weather_entity_id` to your actual HA weather entity.

Find it in Home Assistant: **Developer Tools > States** → search for `weather.`

### "Still no BLE"
**Fix:** Check LVGL buffer size is 15%:
```yaml
build_flags:
  - "-DLVGL_BUFFER_SIZE_PERCENTAGE=15"
```

### "Display shows --"
**Fix:** Wait 5-10 seconds after boot for initial fetch, or press "Refresh Weather" button.

---

## Next Steps

- Read [README-ESPHOME-2025-10-UPGRADE.md](README-ESPHOME-2025-10-UPGRADE.md) for full details
- Customize weather refresh interval
- Update weather pages to use new global variables
- Add more features!

---

## Feature Combinations That Work

| Clock | BLE | Weather | AirQ | WiFi | Free RAM | Works? |
|-------|-----|---------|------|------|----------|--------|
| ✅ | ✅ | ✅ | ❌ | ❌ | ~102KB | ✅ YES! |
| ✅ | ✅ | ❌ | ✅ | ❌ | ~85KB | ✅ Yes |
| ✅ | ✅ | ❌ | ❌ | ✅ | ~80KB | ✅ Yes |
| ✅ | ❌ | ✅ | ✅ | ✅ | ~70KB | ✅ Yes |
| ✅ | ✅ | ✅ | ✅ | ❌ | ~82KB | ✅ Yes |

**Previously impossible:** BLE + Weather is now the easiest combo!
