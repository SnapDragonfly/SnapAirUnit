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


blink_led_handle_t             g_led_handle = NULL;
button_handle_t                g_key_handle = NULL;
esp_event_loop_handle_t        g_evt_handle = NULL;



