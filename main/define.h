
#ifndef __DEFINE_SOURCE_H__
#define __DEFINE_SOURCE_H__

#include "common.h"
#include "version.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WIRELESS_SIMPLIFIED                 0

#define WIFI_SSID_LENGTH                    16
#define WIFI_PASS_LENGTH                    16

#define SPP_SERVER_NAME                     DEVICE_NAME_SNAP_AIR_UNIT
#define SPP_DEVICE_NAME                     DEVICE_NAME_SNAP_AIR_UNIT

#define USB_UART_BAUDRATE                   CONFIG_ESP_CONSOLE_UART_BAUDRATE
#define USB_UART_NUM                        CONFIG_ESP_CONSOLE_UART_NUM

#define MSP_UART_PORT                       UART_NUM_1
#define MSP_RX_BUF_SIZE                    (TTL_BUF_BASIC_SIZE * 2)
#define MSP_TX_BUF_SIZE                    (TTL_BUF_BASIC_SIZE * 4)
#define MSP_RX_EVENT_SIZE                   10

#define CONSOLE_PROMPT_STR                  DEVICE_NAME_SNAP_AIR_UNIT
#define CONSOLE_PROMPT_YES                 "yes"

#define TELLO_RESPONSE_OK                  "ok"
#define TELLO_RESPONSE_ERR                 "error"

#define EVT_STACK_SIZE                      STACK_BUFFER_3K0
#define EVT_QUEUE_SIZE                      5

#define CHANNEL_TAG_LENGTH                  8

#define RESTFUL_API_MAX_URI_HANDLERS        15
#define RESTFUL_API_RESPONSE_OK             TELLO_RESPONSE_OK
#define RESTFUL_API_RESPONSE_INVALID        "invalid"
#define RESTFUL_API_RESPONSE_ERR            TELLO_RESPONSE_ERR



#ifdef __cplusplus
}
#endif

#endif // #ifndef __DEFINE_SOURCE_H__

