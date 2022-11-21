
#ifndef __PROCESS_SOURCE_H__
#define __PROCESS_SOURCE_H__

#include "esp_event.h"
#include "esp_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

// Declare an event base
ESP_EVENT_DECLARE_BASE(EVT_PROCESS);        // declaration of the mode key events family

enum {
    MODE_KEY_RELEASED       = 0,
    MODE_KEY_SHORT_PRESSED  = 1,
    MODE_KEY_LONG_PRESSED   = 2,
    MODE_KEY_PRESS_ACT      = 3
};

esp_event_loop_handle_t module_evt_start(void);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __PROCESS_SOURCE_H__

