#pragma once

#include "esphome/components/api/api_server.h"
#include "esphome/components/api/api_pb2.h"

namespace esphome {
namespace nimble_proxy {

// Helper to broadcast bluetooth advertisements to all connected API clients
inline void send_bluetooth_advertisements_to_clients(api::BluetoothLERawAdvertisementsResponse &resp) {
  if (api::global_api_server == nullptr) {
    return;
  }

  // Access private clients_ member through reflection/offset
  // This is a workaround since APIServer doesn't expose get_clients() in this version
  auto *server = api::global_api_server;

  // Use the public send_message interface on each connection
  // Since we can't access clients_ directly, we'll use a different approach:
  // Create the message and let the server broadcast it

  // For now, just log that we would send
  ESP_LOGD("nimble_proxy", "Would send %d advertisements (API server integration pending)",
           resp.advertisements_len);
}

}  // namespace nimble_proxy
}  // namespace esphome
