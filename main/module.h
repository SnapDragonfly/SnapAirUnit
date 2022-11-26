

#ifndef __MODULE_SOURCE_H__
#define __MODULE_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

// Software components
#define DEBUG_EVT_PROC        1
#define MODULE_EVT_PROC        "evt"

// Hardware components
#define DEBUG_LED_BLINK       0
#define DEBUG_KEY_MODE        1
#define DEBUG_UART            0

#define MODULE_LED_BLINK       "led"
#define MODULE_KEY_MODE        "key"
#define MODULE_UART            "ttl"

// Service Module
#define DEBUG_WIFI_AP         1
#define DEBUG_WIFI_STA        1
#define DEBUG_BT_SPP          0

#define MODULE_WIFI_AP         "sap"
#define MODULE_WIFI_STA        "sta"
#define MODULE_BT_SPP          "spp"

// Application Mode
#define DEBUG_MODE            0
#define DEBUG_HTTP            0


#define MODULE_MODE            "mod"
#define MODULE_HTTP            "http"


esp_err_t snap_sw_module_start(TaskFunction_t pxTaskCode, bool task, const uint32_t stackDepth, const char * const pcName);

#endif // #ifndef __MODULE_SOURCE_H__

