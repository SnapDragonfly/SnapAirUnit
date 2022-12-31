/// @file handle.c

/*
 * idf header files
 */
#include <stdint.h>


/*
 * basic header files
 */
#include "handle.h"

/*
 * module header files
 */
//TBD

/*
 * service header files
 */
#include "blink.h"


blink_led_handle_t             g_mode_handle   = NULL;
blink_led_handle_t             g_status_handle = NULL;
button_handle_t                g_key_handle    = NULL;
esp_event_loop_handle_t        g_evt_handle    = NULL;



