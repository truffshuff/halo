# ESPHome Packages

This directory contains reusable ESPHome configuration packages that can be included in device configurations.

## Available Packages

### WireGuard Package (`wireguard.yaml`)

Provides core WireGuard VPN functionality including:
- WireGuard configuration with all standard settings
- Status and enabled binary sensors
- Latest handshake sensor
- Address text sensor
- Enable/disable switch with automation hooks

**Dependencies:** None

### WireGuard Display Package (`wireguard-display.yaml`)

Provides UI/display support for WireGuard on devices with LVGL displays:
- Global variables for tracking WireGuard display state
- Rendering optimization flags to reduce unnecessary UI updates
- 5-second interval updates for status, address, and handshake display
- Automatic color changes based on connection state
- Formatted handshake time with "time ago" display

**Dependencies:**
- Must be used with `wireguard.yaml`
- Requires `wifi-display.yaml` (shares the same 5s update interval)
- Requires LVGL widgets: `wg_status_label`, `wg_address_label`, `wg_endpoint_label`, `wg_handshake_label`
- Requires colors: `my_green`, `my_gray`
- Requires time component: `sntp_time`
- Requires globals: `boot_complete`, `current_page_index`

### WiFi Display Package (`wifi-display.yaml`)

Provides UI/display support for WiFi status on devices with LVGL displays:
- Global variables for tracking WiFi display state
- Rendering optimization flags to reduce unnecessary UI updates
- 5-second interval updates for WiFi signal, SSID, IP, and connection status
- WiFi logo visibility management
- Signal strength bar and text display

**Dependencies:**
- Requires LVGL widgets: `wifi_stat`, `ym_image`, `wifi_signal_label`, `wifi_signal_bar`, `wifi_ssid_label`, `wifi_ip_label`
- Requires colors: `my_green`, `my_red`
- Requires sensor: `wifi_signal_db`
- Requires globals: `boot_complete`, `current_page_index`

### LVGL WiFi Page Package (`lvgl-wifi-page.yaml`)

Provides the LVGL WiFi statistics page WITHOUT WireGuard information:
- WiFi statistics display (SSID, signal strength, IP address)
- WiFi signal strength bar with gradient indicator (included)
- Navigation button for page transitions
- Optimized layout for WiFi-only displays

**Includes:**
- WiFi signal gradient (red to green, weak to strong)

**Dependencies:**
- Requires `wifi-display.yaml` for update logic
- Requires LVGL display configuration
- Requires colors: `my_white`, `my_gray`
- Requires fonts: `montserrat_18`, `montserrat_20`
- Requires script: `page_transition_cleanup`
- Requires globals: `current_page_index`, `last_auto_rotation_time`

### LVGL WiFi + WireGuard Page Package (`lvgl-wifi-wireguard-page.yaml`)

Provides the LVGL WiFi statistics page WITH WireGuard information:
- WiFi statistics display (SSID, signal strength, IP address)
- WireGuard statistics (status, VPN IP, endpoint, handshake time)
- WiFi signal strength bar with gradient indicator (included)
- Navigation button for page transitions
- Full-featured layout with both WiFi and VPN information

**Includes:**
- WiFi signal gradient (red to green, weak to strong)

**Dependencies:**
- Requires `wifi-display.yaml` for WiFi update logic
- Requires `wireguard-display.yaml` for WireGuard update logic
- Requires LVGL display configuration
- Requires colors: `my_white`, `my_gray`, `my_teal`
- Requires fonts: `montserrat_18`, `montserrat_20`
- Requires script: `page_transition_cleanup`
- Requires globals: `current_page_index`, `last_auto_rotation_time`

### Time Update Script - Basic Package (`time-update-basic.yaml`)

Provides time display update scripts for devices WITHOUT WireGuard:
- Uses Home Assistant time source only
- Updates time displays on AirQ page and vertical clock page
- Includes colon blinking functionality for clock displays
- Optimized rendering with performance tracking

**Dependencies:**
- Requires time component: `ha_time`
- Requires LVGL widgets: `timeVal`, `vclock_hours`, `vclock_minutes`, `vclock_ampm`, `vclock_date`, `vclock_day_top`, `vclock_colon`
- Requires switches: `time_format`, `colon_blink_enabled`
- Requires globals: `time_update_last_started`, `time_update_needs_render`, `time_update_last_text`, `time_update_last_duration`, `colon_blink_state`, `current_page_index`

### Time Update Script - WireGuard Package (`time-update-wireguard.yaml`)

Provides time display update scripts for devices WITH WireGuard:
- Automatically switches between SNTP and Home Assistant time based on WireGuard state
- When WireGuard enabled: uses SNTP time (required for WireGuard)
- When WireGuard disabled: uses Home Assistant time
- Updates time displays on AirQ page and vertical clock page
- Includes colon blinking functionality for clock displays
- Optimized rendering with performance tracking

**Dependencies:**
- Requires `wireguard.yaml` package (for `wireguard_enabled` switch)
- Requires time components: `ha_time`, `sntp_time`
- Requires LVGL widgets: `timeVal`, `vclock_hours`, `vclock_minutes`, `vclock_ampm`, `vclock_date`, `vclock_day_top`, `vclock_colon`
- Requires switches: `time_format`, `colon_blink_enabled`, `wireguard_enabled`
- Requires globals: `time_update_last_started`, `time_update_needs_render`, `time_update_last_text`, `time_update_last_duration`, `colon_blink_state`, `current_page_index`

#### Usage

1. **Include the package in your device configuration:**

```yaml
packages:
  wireguard: !include packages/wireguard.yaml
```

2. **Add required secrets to your `secrets.yaml`:**

```yaml
# WireGuard Configuration
wg_address: "10.0.0.2"              # Your WireGuard interface IP
wg_netmask: "255.255.255.0"         # Your WireGuard netmask
wg_prikey: "your_private_key_here"  # Your WireGuard private key
wg_peerport: 51820                  # Your WireGuard peer port
wg_peerendpt: "1.2.3.4"             # Your WireGuard server IP
wg_pubkey: "server_public_key"      # Your WireGuard server public key
wg_shrdkey: "preshared_key"         # Optional: preshared key
```

3. **Customize allowed IPs (optional):**

If you need different allowed IPs than the default `192.168.1.0/24`, you can override them in your device config:

```yaml
packages:
  wireguard: !include packages/wireguard.yaml

wireguard:
  peer_allowed_ips:
    - 192.168.1.0/24
    - 10.0.0.0/24
```

#### Example Configurations

**1. Basic device without display - WireGuard only:**

```yaml
substitutions:
  device_name: my-sensor

esphome:
  name: ${device_name}
  platform: ESP32
  board: esp32dev

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

# Include WireGuard package
packages:
  wireguard: !include packages/wireguard.yaml

# Your other device-specific configuration...
```

**2. Device with display - WiFi status only (no WireGuard):**

```yaml
substitutions:
  device_name: my-display

esphome:
  name: ${device_name}
  platform: ESP32
  board: esp32dev

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

# Include WiFi display package and basic time update
packages:
  wifi_display: !include packages/wifi-display.yaml
  time_update: !include packages/time-update-basic.yaml

# Your LVGL display configuration
display:
  - platform: ...
    pages:
      - !include packages/lvgl-wifi-page.yaml
      # ... your other pages ...
```

**3. Device with display - WiFi + WireGuard status:**

```yaml
substitutions:
  device_name: my-display-with-vpn

esphome:
  name: ${device_name}
  platform: ESP32
  board: esp32dev

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

# Include all packages for full WiFi + WireGuard display
packages:
  wireguard: !include packages/wireguard.yaml
  wifi_display: !include packages/wifi-display.yaml
  wireguard_display: !include packages/wireguard-display.yaml
  time_update: !include packages/time-update-wireguard.yaml

# Your LVGL display configuration
display:
  - platform: ...
    # ... other display config ...
    pages:
      # Include the WiFi + WireGuard page
      - !include packages/lvgl-wifi-wireguard-page.yaml
      # ... your other pages ...
```


**5. Basic device - WireGuard with custom allowed IPs:**

```yaml
packages:
  wireguard: !include packages/wireguard.yaml

# Override default allowed IPs
wireguard:
  peer_allowed_ips:
    - 192.168.1.0/24
    - 10.0.0.0/24
    - 172.16.0.0/16
```

## Usage Notes

### LVGL Page Packages

The LVGL page packages (`lvgl-wifi-page.yaml` and `lvgl-wifi-wireguard-page.yaml`) are **page definitions** that must be included within your LVGL display's `pages:` section, not in the top-level `packages:` section.

**Correct usage:**
```yaml
display:
  - platform: lvgl
    pages:
      - !include packages/lvgl-wifi-page.yaml  # Correct!
```

**Incorrect usage:**
```yaml
packages:
  wifi_page: !include packages/lvgl-wifi-page.yaml  # Wrong! This won't work
```

### Choosing Between WiFi Page Variants

- Use `lvgl-wifi-page.yaml` if you want **WiFi status only** (no VPN information)
  - Simpler, cleaner layout
  - Less screen space used
  - Perfect for devices without WireGuard

- Use `lvgl-wifi-wireguard-page.yaml` if you want **WiFi + WireGuard status**
  - Shows complete network information
  - Displays VPN connection details
  - Requires both `wifi-display.yaml` and `wireguard-display.yaml` packages

## Creating New Packages

To create a new package:

1. Create a new `.yaml` file in this directory
2. Add configuration sections that can be reused
3. Use `!secret` for sensitive values
4. Document the package in this README
# Test version update
