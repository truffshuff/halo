# Weather Package Documentation

## Overview
The weather functionality has been modularized into a single comprehensive package that can be included or excluded from your Halo configuration.

## Package Structure

### weather-core.yaml
**Purpose**: Core weather functionality
**Contains**:
- Weather globals (page rotation settings, display state tracking)
- Weather icon color update script
- Page rotation controls (switches for enable/disable)
- Page order controls (number inputs)
- Weather gauge control
- Weather diagnostic sensors

**Dependencies**:
- Requires `page-registry` package
- Requires `weather-sensors` package (Home Assistant integrations)
- Requires `weather-pages` package (LVGL UI)

### Weather Pages (Indices)
When the weather package is included, it defines these pages:
- **Page 3**: Weather Forecast (current day forecast with condition icon)
- **Page 4**: Hourly Forecast (24-hour forecast display)
- **Page 5**: Daily Forecast (10-day forecast overview)

### Home Assistant Requirements
The weather package requires the following Home Assistant entities:
- `input_text.forecast_day_N_high_temperature` (N = 1-10)
- `input_text.forecast_day_N_low_temperature` (N = 1-10)
- `input_text.forecast_day_N_condition` (N = 1-10)
- `input_text.forecast_day_N_date` (N = 1-10)
- `input_text.forecast_day_N_precipitation_probability` (N = 1-10)
- Various current weather sensors (outdoor temp, precipitation, etc.)

## Usage

### To Include Weather
```yaml
packages:
  page_registry: !include packages/page-registry.yaml
  weather_core: !include packages/weather-core.yaml
  # weather_sensors: !include packages/weather-sensors.yaml  # Coming soon
  # weather_pages: !include packages/weather-pages.yaml      # Coming soon
```

### To Exclude Weather
Simply don't include the weather packages. Pages 3-5 will not be defined, and the auto page rotation will skip them automatically.

## Migration Notes
- Weather code extracted from lines 89-3200+ of Halo-v1-Core.yaml
- All weather-related globals moved to weather-core package
- Page rotation logic remains in main config but checks for page existence
- LED effects that reference weather conditions may need adjustment

## Future Enhancements
- Split into smaller sub-packages if needed
- Add weather animation effects
- Support for multiple weather sources
