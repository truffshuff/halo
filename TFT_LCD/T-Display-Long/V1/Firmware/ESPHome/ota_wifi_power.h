#pragma once

#include "esp_wifi.h"

inline void halo_set_ota_wifi_power(bool enabled) {
  // ESP-IDF uses quarter-dBm units: 60 = 15 dBm, 34 = 8.5 dBm.
  esp_wifi_set_max_tx_power(enabled ? 60 : 34);
}
