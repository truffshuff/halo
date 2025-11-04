#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"

#ifdef USE_API
#include "esphome/components/api/api_server.h"
#include "esphome/components/api/api_pb2.h"
#endif

// ESP-IDF native NimBLE headers
#include "esp_bt.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include <vector>
#include <array>

#ifndef BLUETOOTH_PROXY_ADVERTISEMENT_BATCH_SIZE
#define BLUETOOTH_PROXY_ADVERTISEMENT_BATCH_SIZE 5
#endif

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
  bool scanning_{false};

#ifdef USE_API
  // Advertisement batching for Home Assistant
  std::array<api::BluetoothLERawAdvertisement, BLUETOOTH_PROXY_ADVERTISEMENT_BATCH_SIZE> adv_buffer_{};
  uint16_t adv_buffer_count_{0};
  uint32_t last_send_time_{0};
#else
  // Dummy variables when API is not available
  uint16_t adv_buffer_count_{0};
  uint32_t last_send_time_{0};
#endif

  void start_scan_();
  void stop_scan_();
  void start_advertising_();
  void setup_services_();

  void send_advertisements_();
  void add_advertisement_(const ble_gap_disc_desc *disc);

  // NimBLE callbacks
  static int gap_event_handler_(struct ble_gap_event *event, void *arg);
  static int scan_callback_(struct ble_gap_event *event, void *arg);
  static void on_sync_();
  static void on_reset_(int reason);
  static void host_task_(void *param);
};

}  // namespace nimble_proxy
}  // namespace esphome
