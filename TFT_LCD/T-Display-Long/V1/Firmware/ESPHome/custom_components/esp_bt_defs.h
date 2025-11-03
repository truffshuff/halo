// NimBLE Compatibility Shim for ESPHome
// This header provides Bluedroid definitions using NimBLE equivalents

#pragma once

#ifdef CONFIG_BT_NIMBLE_ENABLED

#include "host/ble_gap.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

// Map Bluedroid types to NimBLE types
typedef struct ble_gap_conn_desc esp_bd_addr_t[6];

// Address types
#define ESP_BLE_ADDR_TYPE_PUBLIC        BLE_ADDR_PUBLIC
#define ESP_BLE_ADDR_TYPE_RANDOM        BLE_ADDR_RANDOM
#define ESP_BLE_ADDR_TYPE_RPA_PUBLIC    BLE_ADDR_PUBLIC_ID
#define ESP_BLE_ADDR_TYPE_RPA_RANDOM    BLE_ADDR_RANDOM_ID

// Connection parameters
#define ESP_BLE_CONN_INT_MIN            BLE_GAP_INITIAL_CONN_ITVL_MIN
#define ESP_BLE_CONN_INT_MAX            BLE_GAP_INITIAL_CONN_ITVL_MAX
#define ESP_BLE_CONN_LATENCY            0
#define ESP_BLE_CONN_SUP_TIMEOUT        BLE_GAP_SUPERVISION_TIMEOUT

// Scan parameters
#define ESP_BLE_SCAN_TYPE_PASSIVE       BLE_HCI_SCAN_TYPE_PASSIVE
#define ESP_BLE_SCAN_TYPE_ACTIVE        BLE_HCI_SCAN_TYPE_ACTIVE
#define ESP_BLE_SCAN_FILTER_ALLOW_ALL   0

// GAP events
#define ESP_GAP_BLE_SCAN_RESULT_EVT     1
#define ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT 2
#define ESP_GAP_BLE_SCAN_START_COMPLETE_EVT 3
#define ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT 4

// GATT events  
#define ESP_GATTC_CONNECT_EVT           1
#define ESP_GATTC_DISCONNECT_EVT        2
#define ESP_GATTC_SEARCH_CMPL_EVT       3
#define ESP_GATTC_NOTIFY_EVT            4
#define ESP_GATTC_READ_CHAR_EVT         5
#define ESP_GATTC_WRITE_CHAR_EVT        6

// Status codes
#define ESP_GATT_OK                     0
#define ESP_GATT_ERROR                  0x85

// UUIDs
typedef struct {
    uint16_t uuid16;
    uint8_t uuid128[16];
} esp_bt_uuid_t;

#define ESP_UUID_LEN_16     2
#define ESP_UUID_LEN_32     4
#define ESP_UUID_LEN_128    16

// Address structure
typedef struct {
    uint8_t type;
    uint8_t val[6];
} esp_ble_addr_t;

// Scan result type
typedef enum {
    ESP_BLE_EVT_CONN_ADV = 0x00,
    ESP_BLE_EVT_CONN_DIR_ADV = 0x01,
    ESP_BLE_EVT_DISC_ADV = 0x02,
    ESP_BLE_EVT_NON_CONN_ADV = 0x03,
    ESP_BLE_EVT_SCAN_RSP = 0x04,
} esp_ble_evt_type_t;

// Power level
typedef enum {
    ESP_PWR_LVL_N12 = 0,
    ESP_PWR_LVL_N9  = 1,
    ESP_PWR_LVL_N6  = 2,
    ESP_PWR_LVL_N3  = 3,
    ESP_PWR_LVL_N0  = 4,
    ESP_PWR_LVL_P3  = 5,
    ESP_PWR_LVL_P6  = 6,
    ESP_PWR_LVL_P9  = 7,
} esp_power_level_t;

#endif // CONFIG_BT_NIMBLE_ENABLED
