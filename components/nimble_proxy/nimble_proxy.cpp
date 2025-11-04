#include "nimble_proxy.h"

// Undefine NimBLE macros that conflict with ESPHome API enums
#ifdef LOG_LEVEL_NONE
#undef LOG_LEVEL_NONE
#endif
#ifdef LOG_LEVEL_ERROR
#undef LOG_LEVEL_ERROR
#endif
#ifdef LOG_LEVEL_WARN
#undef LOG_LEVEL_WARN
#endif
#ifdef LOG_LEVEL_INFO
#undef LOG_LEVEL_INFO
#endif
#ifdef LOG_LEVEL_DEBUG
#undef LOG_LEVEL_DEBUG
#endif
#ifdef LOG_LEVEL_VERBOSE
#undef LOG_LEVEL_VERBOSE
#endif

#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esp_err.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include <cstring>

#ifdef USE_API
#include "esphome/components/api/api_server.h"
#include "esphome/components/api/api_pb2.h"
#include "api_server_extensions.h"
#endif

extern "C" void ble_store_config_init(void);

namespace esphome {
namespace nimble_proxy {

static const char *const TAG = "nimble_proxy";

// Static pointer for callbacks
static NimBLEProxy *global_nimble_proxy = nullptr;

}  // namespace nimble_proxy

// Create the global_bluetooth_proxy alias in the bluetooth_proxy namespace
// This allows the API component to access nimble_proxy via bluetooth_proxy::global_bluetooth_proxy
namespace bluetooth_proxy {
  nimble_proxy::NimBLEProxy *&global_bluetooth_proxy = nimble_proxy::global_nimble_proxy;
}

namespace nimble_proxy {

void NimBLEProxy::setup() {
  global_nimble_proxy = this;

  if (!this->active_) {
    ESP_LOGI(TAG, "NimBLE Proxy is disabled");
    return;
  }

#ifdef USE_API
  // Allocate advertisement buffer
  if (!this->adv_buffer_allocated_) {
    this->adv_buffer_ = new esphome::api::BluetoothLERawAdvertisement[BLUETOOTH_PROXY_ADVERTISEMENT_BATCH_SIZE];
    this->adv_buffer_allocated_ = true;
  }
#endif

  ESP_LOGI(TAG, "Setting up NimBLE Proxy...");

  // Ensure NVS is initialized (required by Bluetooth stack)
  esp_err_t nvs_ret = nvs_flash_init();
  if (nvs_ret == ESP_ERR_NVS_NO_FREE_PAGES || nvs_ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_LOGW(TAG, "NVS init returned %s, erasing NVS...", esp_err_to_name(nvs_ret));
    ESP_ERROR_CHECK(nvs_flash_erase());
    nvs_ret = nvs_flash_init();
  }
  if (nvs_ret != ESP_OK) {
    ESP_LOGE(TAG, "NVS init failed: %s", esp_err_to_name(nvs_ret));
    return;
  }

  // Release Classic BT memory (ignore error if already released)
  (void) esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);

  // Initialize NimBLE port (handles controller init internally)
  ESP_LOGD(TAG, "Calling nimble_port_init()...");
  esp_err_t ret = nimble_port_init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "nimble_port_init failed: %s", esp_err_to_name(ret));
    return;
  }

  // Configure host callbacks
  ble_hs_cfg.sync_cb = NimBLEProxy::on_sync_;
  ble_hs_cfg.reset_cb = NimBLEProxy::on_reset_;

  // Initialize BLE store config before starting host task
  ESP_LOGD(TAG, "Initializing BLE store config...");
  ble_store_config_init();

  // Initialize GAP/GATT services before starting host task
  ESP_LOGD(TAG, "Initializing GAP/GATT services...");
  ble_svc_gap_init();
  ble_svc_gatt_init();

  // Set device name
  int rc = ble_svc_gap_device_name_set("ESPHome NimBLE Proxy");
  if (rc != 0) {
    ESP_LOGE(TAG, "Error setting device name: %d", rc);
  }

  // Start NimBLE host task (only once)
  if (!this->host_task_started_) {
    ESP_LOGD(TAG, "Starting NimBLE host task...");
    nimble_port_freertos_init(NimBLEProxy::host_task_);
    this->host_task_started_ = true;
  }

  ESP_LOGI(TAG, "NimBLE Proxy setup complete");
}

void NimBLEProxy::on_sync_() {
  ESP_LOGI(TAG, "NimBLE host synced");

  if (global_nimble_proxy != nullptr) {
    global_nimble_proxy->initialized_ = true;
    // Start scanning instead of advertising for BLE proxy functionality
    global_nimble_proxy->start_scan_();
  }
}

void NimBLEProxy::on_reset_(int reason) {
  ESP_LOGW(TAG, "NimBLE host reset; reason=%d", reason);
}

// FreeRTOS task trampoline for NimBLE host
void NimBLEProxy::host_task_(void *param) {
  (void) param;
  nimble_port_run();
  nimble_port_freertos_deinit();
}

void NimBLEProxy::start_scan_() {
  if (this->scanning_) {
    ESP_LOGD(TAG, "Already scanning");
    return;
  }

  struct ble_gap_disc_params scan_params;
  memset(&scan_params, 0, sizeof(scan_params));

  // Configure scan parameters
  scan_params.itvl = 512;  // 320ms (N * 0.625ms)
  scan_params.window = 48;  // 30ms
  scan_params.filter_policy = BLE_HCI_SCAN_FILT_NO_WL;
  scan_params.limited = 0;  // General discovery
  scan_params.passive = 0;  // Active scanning
  scan_params.filter_duplicates = 0;  // Report all advertisements

  ESP_LOGI(TAG, "Starting BLE scan...");
  int rc = ble_gap_disc(BLE_OWN_ADDR_PUBLIC, BLE_HS_FOREVER, &scan_params,
                        NimBLEProxy::scan_callback_, NULL);
  if (rc != 0) {
    ESP_LOGE(TAG, "Error starting scan: %d", rc);
    return;
  }

  this->scanning_ = true;
  ESP_LOGI(TAG, "BLE scan started");
}

void NimBLEProxy::stop_scan_() {
  if (!this->scanning_) {
    return;
  }

  int rc = ble_gap_disc_cancel();
  if (rc != 0) {
    ESP_LOGE(TAG, "Error stopping scan: %d", rc);
    return;
  }

  this->scanning_ = false;
  ESP_LOGI(TAG, "BLE scan stopped");
}

int NimBLEProxy::scan_callback_(struct ble_gap_event *event, void *arg) {
  if (event->type != BLE_GAP_EVENT_DISC) {
    return 0;
  }

  struct ble_gap_disc_desc *disc = &event->disc;

  // Log discovered device
  ESP_LOGD(TAG, "Device found: %02x:%02x:%02x:%02x:%02x:%02x RSSI=%d",
           disc->addr.val[5], disc->addr.val[4], disc->addr.val[3],
           disc->addr.val[2], disc->addr.val[1], disc->addr.val[0],
           disc->rssi);

  // Add to buffer and send to Home Assistant
  if (global_nimble_proxy != nullptr) {
    global_nimble_proxy->add_advertisement_(disc);
  }

  return 0;
}

void NimBLEProxy::start_advertising_() {
  struct ble_gap_adv_params adv_params;
  struct ble_hs_adv_fields fields;
  
  // Configure advertising parameters
  memset(&adv_params, 0, sizeof(adv_params));
  adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;  // Undirected connectable
  adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;  // General discoverable
  
  // Configure advertising data
  memset(&fields, 0, sizeof(fields));
  fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
  
  const char *name = "ESPHome Proxy";
  fields.name = (uint8_t *) name;
  fields.name_len = strlen(name);
  fields.name_is_complete = 1;
  
  int rc = ble_gap_adv_set_fields(&fields);
  if (rc != 0) {
    ESP_LOGE(TAG, "Error setting advertising fields: %d", rc);
    return;
  }
  
  // Start advertising
  rc = ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER,
                         &adv_params, NimBLEProxy::gap_event_handler_, NULL);
  if (rc != 0) {
    ESP_LOGE(TAG, "Error starting advertising: %d", rc);
    return;
  }
  
  ESP_LOGI(TAG, "Advertising started");
}

int NimBLEProxy::gap_event_handler_(struct ble_gap_event *event, void *arg) {
  switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
      ESP_LOGI(TAG, "Connection %s; status=%d",
               event->connect.status == 0 ? "established" : "failed",
               event->connect.status);
      
      if (event->connect.status != 0) {
        // Connection failed, resume advertising
        if (global_nimble_proxy != nullptr) {
          global_nimble_proxy->start_advertising_();
        }
      }
      break;
      
    case BLE_GAP_EVENT_DISCONNECT:
      ESP_LOGI(TAG, "Disconnect; reason=%d", event->disconnect.reason);
      
      // Resume advertising
      if (global_nimble_proxy != nullptr) {
        global_nimble_proxy->start_advertising_();
      }
      break;
      
    case BLE_GAP_EVENT_ADV_COMPLETE:
      ESP_LOGD(TAG, "Advertising complete");
      break;
      
    case BLE_GAP_EVENT_CONN_UPDATE:
      ESP_LOGD(TAG, "Connection updated; status=%d", event->conn_update.status);
      break;
      
    default:
      break;
  }
  
  return 0;
}

void NimBLEProxy::setup_services_() {
  // Services will be added here for full proxy functionality
  // For now, just basic GAP/GATT services from ble_svc_gap/gatt_init()
}

void NimBLEProxy::add_advertisement_(const ble_gap_disc_desc *disc) {
#ifdef USE_API
  if (!this->adv_buffer_allocated_ || this->adv_buffer_ == nullptr) {
    return;
  }

  // Cast the opaque buffer to the correct type
  auto *buffer = static_cast<esphome::api::BluetoothLERawAdvertisement *>(this->adv_buffer_);

  // Build 64-bit MAC address from 6-byte array
  uint64_t address = 0;
  for (int i = 0; i < 6; i++) {
    address |= ((uint64_t) disc->addr.val[i]) << (i * 8);
  }

  // Add to buffer
  auto &adv = buffer[this->adv_buffer_count_];
  adv.address = address;
  adv.rssi = disc->rssi;
  adv.address_type = disc->addr.type;

  // Copy advertisement data (limited to 62 bytes per ESPHome API)
  adv.data_len = std::min((size_t) disc->length_data, sizeof(adv.data));
  if (adv.data_len > 0 && disc->data != nullptr) {
    memcpy(adv.data, disc->data, adv.data_len);
  }

  this->adv_buffer_count_++;

  // Send when buffer is full or after 100ms timeout
  uint32_t now = millis();
  if (this->adv_buffer_count_ >= BLUETOOTH_PROXY_ADVERTISEMENT_BATCH_SIZE ||
      (this->adv_buffer_count_ > 0 && (now - this->last_send_time_) >= 100)) {
    this->send_advertisements_();
  }
#else
  // API not available, just log
  (void) disc;
#endif
}

void NimBLEProxy::send_advertisements_() {
#ifdef USE_API
  if (this->adv_buffer_count_ == 0 || !this->adv_buffer_allocated_ || this->adv_buffer_ == nullptr) {
    return;
  }

  // Cast the opaque buffer to the correct type
  auto *buffer = static_cast<esphome::api::BluetoothLERawAdvertisement *>(this->adv_buffer_);

  esphome::api::BluetoothLERawAdvertisementsResponse resp;
  resp.advertisements_len = this->adv_buffer_count_;

  // Copy buffered advertisements
  for (uint16_t i = 0; i < this->adv_buffer_count_; i++) {
    resp.advertisements[i] = buffer[i];
  }

  // Send to all connected Home Assistant API clients
  send_bluetooth_advertisements_to_clients(this->api_connections_, resp);

  // Reset buffer
  this->adv_buffer_count_ = 0;
  this->last_send_time_ = millis();
#endif
}

void NimBLEProxy::loop() {
  // Send any pending advertisements periodically
  if (this->adv_buffer_count_ > 0) {
    uint32_t now = millis();
    if ((now - this->last_send_time_) >= 100) {
      this->send_advertisements_();
    }
  }
}

uint32_t NimBLEProxy::get_feature_flags() {
  // Define feature flags (matching bluetooth_proxy enums)
  const uint32_t FEATURE_PASSIVE_SCAN = 1 << 0;
  const uint32_t FEATURE_ACTIVE_CONNECTIONS = 1 << 1;
  const uint32_t FEATURE_REMOTE_CACHING = 1 << 2;
  const uint32_t FEATURE_PAIRING = 1 << 3;
  const uint32_t FEATURE_CACHE_CLEARING = 1 << 4;
  const uint32_t FEATURE_RAW_ADVERTISEMENTS = 1 << 5;

  // We support passive scanning and raw advertisements
  // Active connections not yet implemented
  return FEATURE_PASSIVE_SCAN | FEATURE_RAW_ADVERTISEMENTS;
}

std::string NimBLEProxy::get_bluetooth_mac_address_pretty() {
  // Get the BLE device address from NimBLE stack
  // The address is available after NimBLE is initialized
  if (!this->initialized_) {
    return "00:00:00:00:00:00";
  }

  // Get the public device address from NimBLE
  uint8_t addr_type;
  ble_addr_t addr;
  int rc = ble_hs_id_infer_auto(0, &addr_type);
  if (rc != 0) {
    ESP_LOGW(TAG, "Failed to determine BLE address type: %d", rc);
    return "00:00:00:00:00:00";
  }

  rc = ble_hs_id_copy_addr(addr_type, addr.val, NULL);
  if (rc != 0) {
    ESP_LOGW(TAG, "Failed to copy BLE address: %d", rc);
    return "00:00:00:00:00:00";
  }

  // Format as MAC address string (reversed byte order for display)
  char mac_str[18];
  snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
           addr.val[5], addr.val[4], addr.val[3], addr.val[2], addr.val[1], addr.val[0]);
  return std::string(mac_str);
}

void NimBLEProxy::subscribe_api_connection(void *conn, uint32_t flags) {
  // Add this API connection to our list if not already present
  for (auto *existing : this->api_connections_) {
    if (existing == conn) {
      ESP_LOGD(TAG, "API connection %p already subscribed", conn);
      return;
    }
  }
  this->api_connections_.push_back(conn);
  ESP_LOGI(TAG, "API connection %p subscribed, total connections: %d", conn, this->api_connections_.size());
}

void NimBLEProxy::unsubscribe_api_connection(void *conn) {
  // Remove this API connection from our list
  auto it = std::find(this->api_connections_.begin(), this->api_connections_.end(), conn);
  if (it != this->api_connections_.end()) {
    this->api_connections_.erase(it);
    ESP_LOGI(TAG, "API connection %p unsubscribed, remaining connections: %d", conn, this->api_connections_.size());
  }
}

void NimBLEProxy::dump_config() {
  ESP_LOGCONFIG(TAG, "NimBLE Proxy:");
  ESP_LOGCONFIG(TAG, "  Active: %s", YESNO(this->active_));
  ESP_LOGCONFIG(TAG, "  Max Connections: %d", this->max_connections_);
  ESP_LOGCONFIG(TAG, "  Initialized: %s", YESNO(this->initialized_));
  ESP_LOGCONFIG(TAG, "  Host task started: %s", YESNO(this->host_task_started_));
  ESP_LOGCONFIG(TAG, "  API Connections: %d", this->api_connections_.size());
  ESP_LOGCONFIG(TAG, "  BT controller status: %d", (int) esp_bt_controller_get_status());
}

}  // namespace nimble_proxy
}  // namespace esphome
