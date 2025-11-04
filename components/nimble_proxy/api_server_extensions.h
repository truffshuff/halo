#pragma once

#include "esphome/components/api/api_server.h"
#include "esphome/components/api/api_connection.h"
#include "esphome/components/api/api_pb2.h"

namespace esphome {
namespace nimble_proxy {

// Helper to broadcast bluetooth advertisements to all connected API clients
// Takes a vector of API connections (as void*) and sends the advertisement response to each
inline void send_bluetooth_advertisements_to_clients(
    const std::vector<void *> &api_connections,
    api::BluetoothLERawAdvertisementsResponse &resp) {

  if (api_connections.empty()) {
    ESP_LOGV("nimble_proxy", "No API connections to send %d advertisements to", resp.advertisements_len);
    return;
  }

  ESP_LOGD("nimble_proxy", "Sending %d BLE advertisements to %d API client(s)",
           resp.advertisements_len, api_connections.size());

  // Send to each connected API client
  for (void *conn_ptr : api_connections) {
    if (conn_ptr == nullptr) {
      continue;
    }

    // Cast void* back to APIConnection*
    auto *conn = static_cast<api::APIConnection *>(conn_ptr);

    // Send the message using the API connection's send_message method
    if (!conn->send_message(resp, api::BluetoothLERawAdvertisementsResponse::MESSAGE_TYPE)) {
      ESP_LOGW("nimble_proxy", "Failed to send BLE advertisements to API connection %p", conn_ptr);
    }
  }
}

// Helper to send scanner state to all connected API clients
inline void send_scanner_state_to_clients(
    const std::vector<void *> &api_connections,
    api::BluetoothScannerStateResponse &resp) {

  if (api_connections.empty()) {
    ESP_LOGV("nimble_proxy", "No API connections to send scanner state to");
    return;
  }

  ESP_LOGD("nimble_proxy", "Sending scanner state (state=%d, mode=%d) to %d API client(s)",
           resp.state, resp.mode, api_connections.size());

  // Send to each connected API client
  for (void *conn_ptr : api_connections) {
    if (conn_ptr == nullptr) {
      continue;
    }

    // Cast void* back to APIConnection*
    auto *conn = static_cast<api::APIConnection *>(conn_ptr);

    // Send the message using the API connection's send_message method
    if (!conn->send_message(resp, api::BluetoothScannerStateResponse::MESSAGE_TYPE)) {
      ESP_LOGW("nimble_proxy", "Failed to send scanner state to API connection %p", conn_ptr);
    }
  }
}

}  // namespace nimble_proxy
}  // namespace esphome
