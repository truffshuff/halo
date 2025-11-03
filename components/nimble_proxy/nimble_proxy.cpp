#include "nimble_proxy.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace nimble_proxy {

static const char *const TAG = "nimble_proxy";

void NimBLEProxy::setup() {
  ESP_LOGCONFIG(TAG, "Setting up NimBLE Proxy...");
  
  if (!this->active_) {
    ESP_LOGCONFIG(TAG, "  Proxy is disabled");
    return;
  }

  // Initialize NimBLE
  NimBLEDevice::init("ESPHome");
  
  // Set power level
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  
  // Create BLE Server
  this->ble_server_ = NimBLEDevice::createServer();
  this->ble_server_->setCallbacks(new NimBLEServerCallbacks());
  
  // Set connection parameters
  NimBLEDevice::setConnectionParams(
    12,   // min interval (12*1.25ms = 15ms)
    24,   // max interval (24*1.25ms = 30ms)  
    0,    // latency
    51    // timeout (51*10ms = 510ms)
  );

  this->setup_services_();
  this->start_advertising_();
  
  ESP_LOGCONFIG(TAG, "  Max connections: %u", this->max_connections_);
  ESP_LOGCONFIG(TAG, "NimBLE Proxy setup complete");
}

void NimBLEProxy::setup_services_() {
  // Basic GAP service
  NimBLEService *gap_service = this->ble_server_->createService(NimBLEUUID((uint16_t)0x1800));
  
  // Device Name characteristic
  NimBLECharacteristic *name_char = gap_service->createCharacteristic(
    NimBLEUUID((uint16_t)0x2A00),
    NIMBLE_PROPERTY::READ
  );
  name_char->setValue("ESPHome Proxy");
  
  gap_service->start();
  
  // Basic GATT service  
  NimBLEService *gatt_service = this->ble_server_->createService(NimBLEUUID((uint16_t)0x1801));
  gatt_service->start();
  
  ESP_LOGD(TAG, "Services configured");
}

void NimBLEProxy::start_advertising_() {
  NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();
  
  // Add service UUIDs to advertisement
  advertising->addServiceUUID(NimBLEUUID((uint16_t)0x1800)); // GAP
  advertising->addServiceUUID(NimBLEUUID((uint16_t)0x1801)); // GATT
  
  // Set advertising parameters
  advertising->setScanResponse(true);
  advertising->setMinPreferred(0x06);  // 7.5ms
  advertising->setMaxPreferred(0x12);  // 22.5ms
  
  // Start advertising
  advertising->start();
  
  ESP_LOGD(TAG, "Advertising started");
}

void NimBLEProxy::loop() {
  // NimBLE handles everything in background tasks
  // We just monitor connection count
  if (this->active_ && this->ble_server_) {
    uint8_t connected = this->ble_server_->getConnectedCount();
    
    // Log connection changes (optional, can be removed if too verbose)
    static uint8_t last_count = 0;
    if (connected != last_count) {
      ESP_LOGD(TAG, "Connected devices: %u", connected);
      last_count = connected;
    }
  }
}

void NimBLEProxy::dump_config() {
  ESP_LOGCONFIG(TAG, "NimBLE Bluetooth Proxy:");
  ESP_LOGCONFIG(TAG, "  Active: %s", YESNO(this->active_));
  ESP_LOGCONFIG(TAG, "  Max Connections: %u", this->max_connections_);
  if (this->ble_server_) {
    ESP_LOGCONFIG(TAG, "  Connected: %u", this->ble_server_->getConnectedCount());
  }
}

}  // namespace nimble_proxy
}  // namespace esphome
