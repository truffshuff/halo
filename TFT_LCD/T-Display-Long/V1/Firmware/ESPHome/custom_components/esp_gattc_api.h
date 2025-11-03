// NimBLE GATTC API Compatibility Shim
#pragma once

#ifdef CONFIG_BT_NIMBLE_ENABLED

#include "esp_bt_defs.h"
#include "host/ble_gatt.h"

// GATTC callback type
typedef void (*esp_gattc_cb_t)(uint32_t event, uint16_t gattc_if, void *param);

// GATTC interface
typedef uint16_t esp_gatt_if_t;

// GATTC API functions
static inline int esp_ble_gattc_register_callback(esp_gattc_cb_t callback) {
    return 0;
}

static inline int esp_ble_gattc_app_register(uint16_t app_id) {
    return 0;
}

static inline int esp_ble_gattc_open(esp_gatt_if_t gattc_if, uint8_t *remote_bda, bool is_direct) {
    return 0;
}

static inline int esp_ble_gattc_close(esp_gatt_if_t gattc_if, uint16_t conn_id) {
    return 0;
}

#else
#include_next <esp_gattc_api.h>
#endif
