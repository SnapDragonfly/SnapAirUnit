
#ifndef __UART_SOURCE_H__
#define __UART_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/uart.h"

esp_err_t module_ttl_start(void);
esp_err_t ttl_send(uint8_t * buf, int len);


#ifdef __cplusplus
}
#endif

#endif // #ifndef __UART_SOURCE_H__

