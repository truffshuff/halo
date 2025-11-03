// NimBLE Compatibility Shim for ESPHome
// This header provides Bluedroid definitions using NimBLE equivalents

#pragma once

#ifdef CONFIG_BT_NIMBLE_ENABLED

// Include NimBLE headers
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

// Map Bluedroid types to NimBLE types
typedef uint8_t esp_bd_addr_t[6];

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
#define ESP_BLE_SCAN_FILTER_ALLOW_ONLY_WLST 1
#define ESP_BLE_SCAN_FILTER_ALLOW_UND_RPA_DIR 2
#define ESP_BLE_SCAN_FILTER_ALLOW_WLST_PRA_DIR 3

// GAP events
#define ESP_GAP_BLE_SCAN_RESULT_EVT     BLE_GAP_EVENT_DISC
#define ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT BLE_GAP_EVENT_ADV_COMPLETE
#define ESP_GAP_BLE_SCAN_START_COMPLETE_EVT BLE_GAP_EVENT_DISC_COMPLETE
#define ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT BLE_GAP_EVENT_DISC_COMPLETE

// GATT events  
#define ESP_GATTC_CONNECT_EVT           BLE_GAP_EVENT_CONNECT
#define ESP_GATTC_DISCONNECT_EVT        BLE_GAP_EVENT_DISCONNECT
#define ESP_GATTC_SEARCH_CMPL_EVT       BLE_GAP_EVENT_DISC_COMPLETE
#define ESP_GATTC_NOTIFY_EVT            BLE_GAP_EVENT_NOTIFY_RX
#define ESP_GATTC_READ_CHAR_EVT         BLE_GAP_EVENT_NOTIFY_RX
#define ESP_GATTC_WRITE_CHAR_EVT        BLE_GAP_EVENT_MTU

// Status codes
#define ESP_GATT_OK                     0
#define ESP_GATT_ERROR                  0x85
#define ESP_GATT_INVALID_HANDLE         0x01
#define ESP_GATT_READ_NOT_PERMIT        0x02
#define ESP_GATT_WRITE_NOT_PERMIT       0x03

// UUIDs
typedef struct {
    uint16_t len;
    union {
        uint16_t uuid16;
        uint32_t uuid32;
        uint8_t uuid128[16];
    } uuid;
} esp_bt_uuid_t;

#define ESP_UUID_LEN_16     2
#define ESP_UUID_LEN_32     4
#define ESP_UUID_LEN_128    16

// Address structure
typedef struct {
    uint8_t type;
    uint8_t val[6];
} esp_ble_addr_t;

// Scan result structure
typedef struct {
    esp_ble_addr_t bda;
    uint8_t rssi;
    uint8_t ble_adv[31];
    uint8_t ble_adv_len;
    uint8_t scan_rsp[31];
    uint8_t scan_rsp_len;
    uint8_t flag;
    uint8_t num_resps;
    esp_ble_addr_t ble_addr_type;
    uint16_t ble_evt_type;
} esp_ble_gap_cb_param_t;

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

// BLE Advertisement types
#define ESP_BLE_AD_TYPE_FLAG                    0x01
#define ESP_BLE_AD_TYPE_16SRV_PART              0x02
#define ESP_BLE_AD_TYPE_16SRV_CMPL              0x03
#define ESP_BLE_AD_TYPE_32SRV_PART              0x04
#define ESP_BLE_AD_TYPE_32SRV_CMPL              0x05
#define ESP_BLE_AD_TYPE_128SRV_PART             0x06
#define ESP_BLE_AD_TYPE_128SRV_CMPL             0x07
#define ESP_BLE_AD_TYPE_NAME_SHORT              0x08
#define ESP_BLE_AD_TYPE_NAME_CMPL               0x09
#define ESP_BLE_AD_TYPE_TX_PWR                  0x0A
#define ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE   0xFF

#else
// If NimBLE is not enabled, include original Bluedroid headers
#include_next <esp_bt_defs.h>
#endif // CONFIG_BT_NIMBLE_ENABLED
