#include "nimble_proxy.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esp_err.h"
#include "nvs_flash.h"

extern "C" void ble_store_config_init(void);

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
  
  // Release Bluetooth controller memory if not using classic BT (ignore error if already released)
  (void) esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);

  // Determine controller status to avoid double-init
  esp_bt_controller_status_t ctrl_status = esp_bt_controller_get_status();
  ESP_LOGD(TAG, "BT controller status before init: %d", static_cast<int>(ctrl_status));

  // Initialize Bluetooth controller with BLE mode if not already initialized
  if (ctrl_status == ESP_BT_CONTROLLER_STATUS_IDLE) {
    ESP_LOGD(TAG, "Calling esp_bt_controller_init()...");
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_err_t ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Bluetooth controller init failed: %s", esp_err_to_name(ret));
      return;
    }
    ctrl_status = esp_bt_controller_get_status();
    ESP_LOGD(TAG, "BT controller status after init: %d", static_cast<int>(ctrl_status));
  }

  // Enable BLE mode if not already enabled
  ctrl_status = esp_bt_controller_get_status();
  if (ctrl_status != ESP_BT_CONTROLLER_STATUS_ENABLED) {
    ESP_LOGD(TAG, "Enabling BT controller in BLE mode...");
    esp_err_t ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Bluetooth controller enable failed: %s", esp_err_to_name(ret));
      return;
    }
    ESP_LOGD(TAG, "BT controller enabled in BLE mode");
  }
  
  // Initialize NimBLE HCI glue now that controller is enabled
  ESP_LOGD(TAG, "Initializing NimBLE HCI glue...");
  esp_err_t hci_ret = esp_nimble_hci_init();
  if (hci_ret != ESP_OK) {
    ESP_LOGE(TAG, "esp_nimble_hci_init failed: %s", esp_err_to_name(hci_ret));
    return;  // Abort setup to avoid host task crash
  }
  
  // Initialize NimBLE host (HCI glue is handled by NimBLE port when controller is enabled)
  ESP_LOGD(TAG, "Calling nimble_port_init()...");
  nimble_port_init();
  
  // Configure host callbacks
  ble_hs_cfg.sync_cb = NimBLEProxy::on_sync_;
  ble_hs_cfg.reset_cb = NimBLEProxy::on_reset_;
  
  // Initialize services and storage before starting host task
  ESP_LOGD(TAG, "Initializing GAP/GATT services...");
  ble_svc_gap_init();
  ble_svc_gatt_init();

  ESP_LOGD(TAG, "Initializing BLE store config...");
  ble_store_config_init();

  // Set device name (can be set before advertising starts)
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
  ESP_LOGCONFIG(TAG, "  Host task started: %s", YESNO(this->host_task_started_));
  ESP_LOGCONFIG(TAG, "  BT controller status: %d", (int) esp_bt_controller_get_status());
}

}  // namespace nimble_proxy
}  // namespace esphome
