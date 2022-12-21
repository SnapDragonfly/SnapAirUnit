

#ifndef __CONFIG_SOURCE_H__
#define __CONFIG_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif


// for wifi AP mode
#define FACTORY_ESP_WIFI_AP_SSID            CONFIG_ESP_WIFI_AP_SSID
#define FACTORY_ESP_WIFI_AP_PASS            CONFIG_ESP_WIFI_AP_PASSWORD

#define FACTORY_ESP_WIFI_CHANNEL            CONFIG_ESP_WIFI_CHANNEL
#define FACTORY_MAX_STA_CONN                CONFIG_ESP_MAX_STA_CONN

// for wifi station mode
#define FACTORY_ESP_WIFI_STA_SSID           CONFIG_ESP_WIFI_STA_SSID
#define FACTORY_ESP_WIFI_STA_PASS           CONFIG_ESP_WIFI_STA_PASSWORD

#define FACTORY_ESP_MAXIMUM_RETRY           CONFIG_ESP_MAXIMUM_RETRY

// for keys
#define KEY_MODE                            CONFIG_KEY_MODE

#define CONFIG_KEY_RESERVE_TIME_IN_MS      (CONFIG_KEY_PRESS_RESERVE*1000)

// for msp uart port
#define MSP_UART_TXD                        CONFIG_MSP_TTL_TXD
#define MSP_UART_RXD                        CONFIG_MSP_TTL_RXD

// for UDP client port
#define STATUS_PORT                         CONFIG_STATUS_SERVER_PORT
#define CONTROL_PORT                        CONFIG_CONTROL_SERVER_PORT


#ifdef __cplusplus
}
#endif

#endif // #ifndef __CONFIG_SOURCE_H__


