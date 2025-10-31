# Weather Sensors Memory Optimization Guide

## Problem Analysis

Your weather-sensors.yaml file contains **125 Home Assistant sensor entities**, each consuming:
- ~200-500 bytes for the sensor object
- ~100-300 bytes for string buffers in `on_value` callbacks
- ~200-400 bytes for API subscription and update queues
- **Total: ~500-1200 bytes per sensor = 62KB-150KB heap usage!**

## Solution: Multiple Optimization Strategies

### Strategy 1: Mark Display-Only Sensors as Internal (Recommended)

Since these sensors are only used to display data on the device (not control anything in HA), mark them as `internal: true`. This prevents them from being published back to Home Assistant, saving memory.

**Example:**
```yaml
- platform: homeassistant
  name: "Forecast Day 1 High Temp"
  entity_id: input_text.forecast_day_1_high_temperature
  id: forecast_day1_high_temp
  internal: true  # <-- Add this to prevent republishing to HA
  on_value:
    then:
      - lvgl.label.update:
          id: lbl_weather_forecast_temphi
          text: !lambda |-
            char buffer[16];
            if (isnan(x)) {
              snprintf(buffer, sizeof(buffer), "H: --°");
            } else {
              snprintf(buffer, sizeof(buffer), "H: %.0f°", x);
            }
            return buffer;
```

**Memory Savings:** ~150-300 bytes per sensor × 125 sensors = **18KB-37KB saved**

---

### Strategy 2: Batch UI Updates (Reduce Cascade Rendering)

Instead of updating LVGL labels immediately in `on_value`, set a flag and batch updates every 2-5 seconds. This is already partially implemented in your core config with `sensor_ui_dirty` flag.

**Example:**
```yaml
- platform: homeassistant
  name: "Forecast Day 1 High Temp"
  entity_id: input_text.forecast_day_1_high_temperature
  id: forecast_day1_high_temp
  internal: true
  on_value:
    then:
      # Don't update LVGL immediately - just store the value
      # Let the batched interval handler update the UI
      - lambda: 'id(sensor_ui_dirty) = true;'
```

Then in a central interval (in weather-core.yaml or weather-pages.yaml):
```yaml
interval:
  - interval: 3s
    then:
      - if:
          condition:
            lambda: 'return id(sensor_ui_dirty) && id(boot_complete);'
          then:
            - lambda: |-
                // Update all weather labels at once
                id(lbl_weather_forecast_temphi).set_text(
                  (std::string("H: ") + to_string((int)id(forecast_day1_high_temp).state) + "°").c_str()
                );
                // ... update other labels ...
                id(sensor_ui_dirty) = false;
```

**Memory Savings:** Reduces LVGL render queue buildup, saves ~10-20KB heap during updates

---

### Strategy 3: Use Text Sensors for String Data

Your forecast uses `input_text` entities in Home Assistant but imports them as numeric sensors. Text sensors are more memory-efficient for string data.

**Example:**
```yaml
text_sensor:
  - platform: homeassistant
    name: "Forecast Day 1 Condition"
    entity_id: input_text.forecast_day_1_condition
    id: forecast_day1_condition
    internal: true
    on_value:
      then:
        - lvgl.label.update:
            id: lbl_forecast_day1_condition
            text: !lambda 'return x.c_str();'
```

**Memory Savings:** ~50-100 bytes per text field × 50+ text fields = **2.5KB-5KB saved**

---

### Strategy 4: Reduce Update Frequency

Add `update_interval` to sensors that don't need real-time updates (weather doesn't change every second).

**Example:**
```yaml
- platform: homeassistant
  name: "Forecast Day 1 High Temp"
  entity_id: input_text.forecast_day_1_high_temperature
  id: forecast_day1_high_temp
  internal: true
  # Weather forecasts don't change often
  # Throttle updates to reduce API traffic and memory churn
  filters:
    - throttle: 60s  # Update at most once per minute
```

**Memory Savings:** Reduces API queue buildup, saves ~5-10KB during heavy update periods

---

### Strategy 5: Lazy Load Forecast Days

Only load forecast data for days that are currently visible on screen. Use conditionals to enable/disable sensor subscriptions.

**Example (Advanced):**
```yaml
# Only subscribe to Day 4-5 sensors when that page is active
interval:
  - interval: 1s
    then:
      - if:
          condition:
            lambda: 'return id(current_page_index) == 5;'  # Days 4-5 page
          then:
            # Enable Day 4-5 sensor subscriptions
            - lambda: 'id(forecast_day4_high_temp).set_update_interval(1000);'
          else:
            # Disable when not viewing this page
            - lambda: 'id(forecast_day4_high_temp).set_update_interval(0);'
```

**Memory Savings:** Only active page sensors consume memory, saves ~30-50KB when viewing simple pages

---

## ESP-IDF Configuration (Already Applied)

The following ESP-IDF settings have been added to Halo-v1-Core.yaml to move sensor allocations to PSRAM:

```yaml
CONFIG_ESP_MAIN_TASK_STACK_SIZE: "8192"  # Increased for 125+ sensors
CONFIG_SPIRAM_MALLOC_RESERVE_INTERNAL: "32768"  # Keep 32KB internal for critical ops
CONFIG_LWIP_MAX_SOCKETS: "16"  # Support heavy API traffic
CONFIG_LWIP_SO_RCVBUF: "y"  # Optimize socket buffers
```

These settings ensure that:
- Sensor object allocations go to PSRAM (>16KB threshold)
- API communication buffers use PSRAM
- Task stacks for sensor processing use PSRAM

---

## Recommended Implementation Order

1. **Quick Win:** Add `internal: true` to all weather sensors (5 minutes, 18-37KB savings)
2. **Medium Effort:** Add `throttle: 60s` filters to forecast sensors (10 minutes, 5-10KB savings)
3. **Advanced:** Implement batched UI updates (30 minutes, 10-20KB savings)
4. **Optional:** Lazy load forecast days (1 hour, 30-50KB savings)

---

## Expected Results

With just strategies 1-2 implemented:
- **Internal heap savings:** 25-45KB
- **PSRAM usage:** Sensor objects moved to external RAM
- **Smoother rendering:** Fewer UI cascade updates
- **Stable operation:** With BLE, WiFi, sensors, and display all enabled

---

## Testing

After applying optimizations, monitor with memory_stats.yaml:
```yaml
sensor:
  - platform: template
    name: "Free Heap"
    lambda: 'return heap_caps_get_free_size(MALLOC_CAP_INTERNAL);'
    update_interval: 10s

  - platform: template
    name: "Free PSRAM"
    lambda: 'return heap_caps_get_free_size(MALLOC_CAP_SPIRAM);'
    update_interval: 10s
```

Target values:
- **Free Internal Heap:** >120KB (with all features enabled)
- **Free PSRAM:** >7MB (plenty of room for growth)
