#ifndef __HANDLE_SOURCE_H__
#define __HANDLE_SOURCE_H__

#include "blink.h"
#include "iot_button.h"
#include "esp_event_base.h"

#ifdef __cplusplus
extern "C" {
#endif

extern blink_led_handle_t             g_led_handle;
extern button_handle_t                g_key_handle;
extern esp_event_loop_handle_t        g_evt_handle;
extern uint32_t                       g_esp_ssp_handle;


#ifdef __cplusplus
}
#endif

#endif // #ifndef __HANDLE_SOURCE_H__

