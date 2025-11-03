#include "nimble_proxy.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace nimble_proxy {

static const char *const TAG = "nimble_proxy";

// Static pointer for callbacks
static NimBLEProxy *global_nimble_proxy = nullptr;

void NimBLEProxy::setup() {
  global_nimble_proxy = this;
  
  if (!this->active_) {
    ESP_LOGI(TAG, "NimBLE Proxy is disabled");
    return;
  }

  ESP_LOGI(TAG, "Setting up NimBLE Proxy...");
  
  // Initialize NimBLE host
  ESP_ERROR_CHECK(esp_nimble_hci_and_controller_init());
  nimble_port_init();
  
  // Configure host callbacks
  ble_hs_cfg.sync_cb = NimBLEProxy::on_sync_;
  ble_hs_cfg.reset_cb = NimBLEProxy::on_reset_;
  
  // Initialize services
  ble_svc_gap_init();
  ble_svc_gatt_init();
  
  // Set device name
  int rc = ble_svc_gap_device_name_set("ESPHome NimBLE Proxy");
  if (rc != 0) {
    ESP_LOGE(TAG, "Error setting device name: %d", rc);
  }
  
  // Start NimBLE host task
  nimble_port_freertos_init(NimBLEProxy::host_task_);
  
  ESP_LOGI(TAG, "NimBLE Proxy setup complete");
}

void NimBLEProxy::on_sync_() {
  ESP_LOGI(TAG, "NimBLE host synced");
  
  if (global_nimble_proxy != nullptr) {
    global_nimble_proxy->initialized_ = true;
    global_nimble_proxy->start_advertising_();
  }
}

void NimBLEProxy::on_reset_(int reason) {
  ESP_LOGW(TAG, "NimBLE host reset, reason: %d", reason);
  if (global_nimble_proxy != nullptr) {
    global_nimble_proxy->initialized_ = false;
  }
}

void NimBLEProxy::host_task_(void *param) {
  ESP_LOGI(TAG, "NimBLE host task started");
  
  // This function will run the nimble host
  nimble_port_run();
  
  // nimble_port_run() should never return, but just in case:
  nimble_port_freertos_deinit();
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

void NimBLEProxy::loop() {
  // NimBLE runs in its own FreeRTOS task, nothing needed here
}

void NimBLEProxy::dump_config() {
  ESP_LOGCONFIG(TAG, "NimBLE Proxy:");
  ESP_LOGCONFIG(TAG, "  Active: %s", YESNO(this->active_));
  ESP_LOGCONFIG(TAG, "  Max Connections: %d", this->max_connections_);
  ESP_LOGCONFIG(TAG, "  Initialized: %s", YESNO(this->initialized_));
}

}  // namespace nimble_proxy
}  // namespace esphome
