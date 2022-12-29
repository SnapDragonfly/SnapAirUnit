
#ifndef __BTSPP_SOURCE_H__
#define __BTSPP_SOURCE_H__

#include "esp_event.h"
#include "esp_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

void bt_spp_start(void);
void bt_spp_stop(void);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __BTSPP_SOURCE_H__

