
#ifndef __BTSPP_SOURCE_H__
#define __BTSPP_SOURCE_H__

#include "esp_event.h"
#include "esp_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

void bt_init_spp(void);
void ttl_uart_init(void);
void bt_deinit_spp(void);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __BTSPP_SOURCE_H__

