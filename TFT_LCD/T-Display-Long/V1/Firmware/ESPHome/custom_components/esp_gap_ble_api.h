// NimBLE GAP API Compatibility Shim
#pragma once

#ifdef CONFIG_BT_NIMBLE_ENABLED

#include "esp_bt_defs.h"
#include "host/ble_gap.h"

// GAP callback type
typedef void (*esp_gap_ble_cb_t)(uint32_t event, void *param);

// GAP API functions mapped to NimBLE
static inline int esp_ble_gap_register_callback(esp_gap_ble_cb_t callback) {
    return 0; // NimBLE uses different callback mechanism
}

static inline int esp_ble_gap_start_scanning(uint32_t duration) {
    return ble_gap_disc(BLE_OWN_ADDR_PUBLIC, duration, NULL, NULL, NULL);
}

static inline int esp_ble_gap_stop_scanning(void) {
    return ble_gap_disc_cancel();
}

static inline int esp_ble_gap_set_scan_params(void *scan_params) {
    return 0; // Handled by NimBLE configuration
}

#else
#include_next <esp_gap_ble_api.h>
#endif
