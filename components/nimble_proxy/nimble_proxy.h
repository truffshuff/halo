#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"

// ESP-IDF native NimBLE headers
#include "esp_bt.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "esp_nimble_hci.h"

namespace esphome {
namespace nimble_proxy {

class NimBLEProxy : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_BLUETOOTH; }

  void set_active(bool active) { this->active_ = active; }
  void set_max_connections(uint8_t max_connections) { this->max_connections_ = max_connections; }

 protected:
  bool active_{true};
  uint8_t max_connections_{3};
  bool initialized_{false};
  bool host_task_started_{false};
  
  void start_advertising_();
  void setup_services_();
  
  // NimBLE callbacks
  static int gap_event_handler_(struct ble_gap_event *event, void *arg);
  static void on_sync_();
  static void on_reset_(int reason);
  static void host_task_(void *param);
};

}  // namespace nimble_proxy
}  // namespace esphome
