

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
#define FACTORY_ESP_WIFI_AP_IP_A            192
#define FACTORY_ESP_WIFI_AP_IP_B            168
#define FACTORY_ESP_WIFI_AP_IP_SEGMENT      CONFIG_ESP_WIFI_AP_IP_SEGMENT
#define FACTORY_ESP_WIFI_AP_IP_D            1
#define FACTORY_ESP_WIFI_AP_IP_MASK         255


#define FACTORY_ESP_MAXIMUM_RETRY           CONFIG_ESP_MAXIMUM_RETRY
#define FACTORY_ESP_MAXIMUM_RETRY_TIMES     CONFIG_ESP_MAXIMUM_RETRY_TIMES

// for keys
#define KEY_MODE                            CONFIG_KEY_MODE

#define CONFIG_KEY_RESERVE_TIME_IN_MS      (CONFIG_KEY_PRESS_RESERVE*1000)

// for pass through mode
#if defined(CONFIG_PASS_THROUGH_UART)
    #define PASS_THROUGH_UART
    //#pragma message(PASS_THROUGH_UART defined)
#elif defined(CONFIG_PASS_THROUGH_HY)
    #define PASS_THROUGH_HY
    //#error PASS_THROUGH_HY configure NOT supported!!!
#else /* CONFIG_PASS_THROUGH_MSP */
    #define PASS_THROUGH_MSP
    //#pragma message(PASS_THROUGH_MSP defined)
#endif /* CONFIG_UART_PASS_THROUGH */

// for msp uart port
#define MSP_UART_TXD                        CONFIG_MSP_TTL_TXD
#define MSP_UART_RXD                        CONFIG_MSP_TTL_RXD

// for UDP client port
#define STATUS_PORT                         CONFIG_STATUS_SERVER_PORT
#define CONTROL_PORT                        CONFIG_CONTROL_SERVER_PORT

// for leds
#define MODE_GPIO                           CONFIG_BLINK_MODE_GPIO
#define STATUS_GPIO                         CONFIG_BLINK_STATUS_GPIO

#define BLINK_LED_LEVEL_ON_LOW              CONFIG_BLINK_LED_LEVEL_ON_LOW
#define BLINK_LED_LEVEL_ON_HIGH             CONFIG_BLINK_LED_LEVEL_ON_HIGH

// for RF mode
#define ESP_RF_MODE                         CONFIG_ESP_RF_MODE


#ifdef __cplusplus
}
#endif

#endif // #ifndef __CONFIG_SOURCE_H__


