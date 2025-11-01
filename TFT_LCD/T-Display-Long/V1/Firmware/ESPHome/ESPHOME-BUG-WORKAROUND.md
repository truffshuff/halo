# ESPHome + Home Assistant 2025.10.x Compatibility Issue

## Problem Summary

**ESPHome 2025.10.3's `homeassistant.action` with `capture_response: true` does not work with Home Assistant 2025.10.x's new action format, specifically for `weather.get_forecasts`.**

## Symptoms

- ESPHome config validates successfully ✅
- `weather.get_forecasts` works when called manually in HA Developer Tools ✅
- When ESPHome calls the action, Home Assistant logs show:
  ```
  ERROR (MainThread) [homeassistant] Error doing job: Task exception was never retrieved (None)
  ```
- Neither `on_success` nor `on_error` handlers are triggered in ESPHome
- No detailed error information is available

## Root Cause

Home Assistant 2025.10.x changed the action/service call format:
- Actions now use `target:` for entity selection and `data:` for parameters
- ESPHome's `homeassistant.action` doesn't support the `target:` parameter
- The old combined `data:` format is incompatible with the new API

## Attempted Fixes (All Failed)

1. ❌ Using `data:` with both `entity_id` and `type`
2. ❌ Using `data_template:` instead of `data:`
3. ❌ Separating `target:` and `data:` (ESPHome doesn't support `target:`)
4. ❌ Adding `return_response: true` parameter
5. ❌ Using quoted vs unquoted entity IDs

## Working Workaround: HTTP REST API

Instead of using `homeassistant.action`, we directly call Home Assistant's REST API using `http_request.post`.

### Configuration Required

1. **Add `http_request` component to your ESPHome config:**
   ```yaml
   http_request:
     useragent: esphome/halo
     timeout: 10s
   ```

2. **Create a long-lived access token in Home Assistant:**
   - Go to your Profile → Security → Long-Lived Access Tokens
   - Click "Create Token"
   - Give it a name (e.g., "ESPHome Weather")
   - Copy the token

3. **Add to your `secrets.yaml`:**
   ```yaml
   ha_url: "http://homeassistant.local:8123"  # Your HA URL (use IP if .local doesn't work)
   ha_token: "your_long_lived_access_token_here"
   ```

4. **Add substitutions to your device YAML:**
   ```yaml
   substitutions:
     ha_url: !secret ha_url
     ha_token: !secret ha_token
   ```

### How It Works

The workaround calls the HA REST API endpoint directly:
```
POST http://homeassistant.local:8123/api/services/weather/get_forecasts
Headers:
  Authorization: Bearer <token>
  Content-Type: application/json
Body:
  {
    "entity_id": "weather.hhut",
    "type": "daily"
  }
```

This returns the forecast data in the correct format, which we parse using ArduinoJson.

## ESPHome GitHub Issue

**TODO:** File an issue at https://github.com/esphome/issues

Suggested title: "homeassistant.action with capture_response doesn't work with HA 2025.10.x weather.get_forecasts"

Include:
- ESPHome version: 2025.10.3
- Home Assistant version: 2025.10.x
- The symptoms described above
- Link to this workaround

## When This Can Be Removed

Once ESPHome adds support for:
1. The `target:` parameter in `homeassistant.action`, OR
2. Proper handling of HA 2025.10.x's new action response format

Then we can revert to using `homeassistant.action` directly.

## Files Modified

- `Halo-v1-Core.yaml`: Added `http_request` component and rewrote `fetch_weather_data` script
- `halo-v1-79e384.yaml`: Added `ha_url` and `ha_token` substitutions
- `secrets.yaml`: (User must add) HA URL and token

---

**Last Updated:** November 1, 2025  
**Status:** Workaround implemented and working
