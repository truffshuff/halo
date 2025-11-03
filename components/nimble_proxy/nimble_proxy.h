#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEAdvertising.h>

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
  NimBLEServer *ble_server_{nullptr};
  
  void start_advertising_();
  void setup_services_();
};

}  // namespace nimble_proxy
}  // namespace esphome
