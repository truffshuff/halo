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

# Include WiFi display package
packages:
  wifi_display: !include packages/wifi-display.yaml

# Your LVGL display configuration with required widgets...
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

# Your LVGL display configuration with required widgets...
```

**4. Basic device - WireGuard with custom allowed IPs:**

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

## Creating New Packages

To create a new package:

1. Create a new `.yaml` file in this directory
2. Add configuration sections that can be reused
3. Use `!secret` for sensitive values
4. Document the package in this README
