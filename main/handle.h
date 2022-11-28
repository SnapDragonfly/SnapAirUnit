#ifndef __HANDLE_SOURCE_H__
#define __HANDLE_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "iot_button.h"
#include "blink.h"


extern blink_led_handle_t             g_led_handle;
extern button_handle_t                g_key_handle;
extern esp_event_loop_handle_t        g_evt_handle;


#ifdef __cplusplus
}
#endif

#endif // #ifndef __HANDLE_SOURCE_H__

