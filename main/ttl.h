
#ifndef __UART_SOURCE_H__
#define __UART_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/uart.h"


#define TTL_UART_NUM        (UART_NUM_1)

void module_ttl_start(uart_port_t uart_num);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __UART_SOURCE_H__

