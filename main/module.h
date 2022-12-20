

#ifndef __MODULE_SOURCE_H__
#define __MODULE_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"


// Software components
#define DEBUG_EVT_PROC            1
#define DEBUG_SPIFFS              1
#define DEBUG_CMD_NVS             1
#define DEBUG_CMD_SAU             1
#define DEBUG_FACTORY_SETTING     1
#define DEBUG_CMD_UDP             1


#define MODULE_EVT_PROC            "evt"
#define MODULE_SPIFFS              "ffs"
#define MODULE_CMD_NVS             "nvs"
#define MODULE_CMD_SAU             "sau"
#define MODULE_FACTORY_SETTING     "fac"
#define MODULE_CMD_UDP             "udp"


// Hardware components
#define DEBUG_LED_BLINK           0
#define DEBUG_KEY_MODE            0
#define DEBUG_UART                0

#define MODULE_LED_BLINK           "led"
#define MODULE_KEY_MODE            "key"
#define MODULE_UART                "ttl"

// Service Module
#define DEBUG_WIFI_AP             0
#define DEBUG_WIFI_STA            0
#define DEBUG_BT_SPP              0

#define MODULE_WIFI_AP             "sap"
#define MODULE_WIFI_STA            "sta"
#define MODULE_BT_SPP              "spp"

// Application Mode
#define DEBUG_MODE                0
#define DEBUG_HTTP                0
#define DEBUG_UDP_SRV             0
#define DEBUG_UDP_CLT             0
#define DEBUG_MSP_PROTO           0
#define DEBUG_TELLO_PROTO         0
#define DEBUG_CONSOLE             0


#define MODULE_MODE                "mod"
#define MODULE_HTTP                "http"
#define MODULE_UDP_SRV             "usrv"
#define MODULE_UDP_CLT             "uclt"
#define MODULE_MSP_PROTO           "msp"
#define MODULE_TELLO_PROTO         "tel"
#define MODULE_CONSOLE             "cons"


esp_err_t snap_sw_module_start(TaskFunction_t pxTaskCode, bool task, const uint32_t stackDepth, const char * const pcName);

#endif // #ifndef __MODULE_SOURCE_H__

