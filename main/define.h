
#ifndef __DEFINE_SOURCE_H__
#define __DEFINE_SOURCE_H__

#include "common.h"
#include "version.h"

#ifdef __cplusplus
extern "C" {
#endif


#define SSID_LENGTH                    16
#define PASS_LENGTH                    16

#define MSP_UART_PORT                  UART_NUM_1
#define MSP_RX_BUF_SIZE               (TTL_BUF_BASIC_SIZE * 2)
#define MSP_TX_BUF_SIZE               (TTL_BUF_BASIC_SIZE * 4)
#define MSP_RX_EVENT_SIZE              10

#ifdef __cplusplus
}
#endif

#endif // #ifndef __DEFINE_SOURCE_H__

