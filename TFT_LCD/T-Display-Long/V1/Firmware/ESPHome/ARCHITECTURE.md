# Halo V1 Package Architecture

This document explains the design of the modular package system and how to extend it.

## System Overview

The Halo V1 configuration system is built on the principle of **modular, feature-independent packages**. Each feature (AirQ, WiFi, Weather, etc.) has:

- A **core package** (e.g., `airq-core.yaml`) that defines sensors, globals, and page rotation controls
- A **display package** (e.g., `airq-display.yaml`) that defines LVGL widgets for the page
- A **stub package** (e.g., `airq-stubs.yaml`) that provides minimal globals to prevent compilation errors when the feature is disabled

## Package Types

### 1. Core Packages

**Purpose:** Define feature-specific sensors, controls, and globals

**Examples:**
- `airq-core.yaml` - SCD40, MICS-4514, BME280, SEN55 sensors
- `wifi-core.yaml` - WiFi page rotation controls, WiFi signal sensor
- `weather-core.yaml` - Weather API integration

**Contains:**
- Sensor definitions
- Global variables for page rotation and state tracking
- Control switches and numbers
- API services for calibration, etc.

**Does NOT contain:**
- LVGL widget definitions (those go in display packages)
- Page definitions (those go in lvgl-pages packages)

### 2. Display Packages

**Purpose:** Define rendering logic for LVGL widgets

**Examples:**
- `airq-display.yaml` - AirQ page widget definitions and update intervals
- `wifi-display.yaml` - WiFi status page update logic
- `vertical-clock-display.yaml` - Clock page widget definitions

**Contains:**
- LVGL widget definitions
- Interval-based update logic
- Color and styling rules
- Gradient definitions

**Dependencies:**
- Requires the matching core package to be loaded
- Requires global variables defined in the core package
- Requires color definitions from the main config

### 3. Stub Packages

**Purpose:** Provide minimal globals when a feature is disabled

**Examples:**
- `airq-stubs.yaml` - Provides stub `page_rotation_AirQ_enabled` global
- `wifi-stubs.yaml` - Provides stub `page_rotation_wifi_enabled` global

**Why they're needed:**
- Some packages reference page rotation globals (e.g., `id(page_rotation_AirQ_enabled)`)
- If a feature is disabled, these globals still need to exist but with disabled values
- Without stubs, other packages would get "Couldn't find ID" errors

## Common Pitfalls and How to Avoid Them

### Pitfall 1: Hard-coded Widget References

❌ **Bad:**
```yaml
interval:
  - interval: 5s
    then:
      - lvgl.label.update:
          id: timeVal  # Fails if AirQ page doesn't exist!
          text: "12:00"
```

✅ **Good:**
```yaml
interval:
  - interval: 5s
    then:
      - if:
          condition:
            lambda: 'return id(current_page_index) == 1;'  # Check first!
          then:
            - lvgl.label.update:
                id: timeVal
                text: "12:00"
```

### Pitfall 2: Missing Index Guard

❌ **Bad:**
```yaml
interval:
  - interval: 5s
    then:
      - if:
          condition:
            wifi.connected:
          then:
            - lvgl.widget.show:
                id: ym_image  # Might not exist in wifi-only mode!
```

✅ **Good:**
```yaml
interval:
  - interval: 5s
    then:
      - if:
          condition:
            lambda: 'return id(boot_complete) && id(current_page_index) == 1;'  # Only show if on AirQ page
          then:
            - lvgl.widget.show:
                id: ym_image
```

### Pitfall 3: Wrong Time Update Package

❌ **Bad for WiFi-only:**
```yaml
time_update_script: !include packages/time-update-basic.yaml
# Tries to update timeVal widget on AirQ page that doesn't exist!
```

✅ **Good for WiFi-only:**
```yaml
time_update_script: !include packages/time-update-wifi-only.yaml
# Only updates vertical clock widgets
```

## Design Principles

### 1. Separation of Concerns

- **Core packages:** Define data sources and controls
- **Display packages:** Define rendering and visual updates
- **Pages packages:** Assemble everything into a complete UI
- **Time update packages:** Handle timing and page transitions

### 2. Feature Independence

- Enabling/disabling a feature should only require:
  - Setting the feature toggle
  - Including the matching -core or -stubs package
  - Selecting the matching pages package
  - Selecting the matching time-update package

### 3. Safe Defaults

- Stub packages provide safe fallback values for disabled features
- Global variables always exist, even if set to "disabled"
- Page rotation logic handles disabled pages gracefully

### 4. Always Guard Widget Updates

- ALWAYS check the current page index before updating widgets
- ALWAYS check if a feature is enabled before referencing its globals
- Use lambda conditions to make runtime checks
