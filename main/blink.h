
#ifndef __BLINK_SOURCE_H__
#define __BLINK_SOURCE_H__

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LED_BLINK_NONE            = 0,
    LED_BLINK_SLOW            = 1,
    LED_BLINK_FAST            = 2,
    LED_BLINK_WARN            = 3,
    LED_BLINK_ERROR           = 4,

    LED_BLINK_NULL,
} led_blink_t;


typedef enum {
    LED_STATUS_NONE           = LED_BLINK_NONE,  // Connection Established
    LED_STATUS_SLOW           = LED_BLINK_SLOW,  // Wait for Connection
    LED_STATUS_FAST           = LED_BLINK_FAST,  // Timeout for Connection
    LED_STATUS_WARN           = LED_BLINK_WARN,  // Wait for Binding
    LED_STATUS_ERROR          = LED_BLINK_ERROR,  // Error, exception

    LED_STATUS_NULL,
} led_status_t;

typedef enum {
    LED_MODE_WIFI_AP          = LED_BLINK_NONE,  // WiFi AP
    LED_MODE_WIFI_STA         = LED_BLINK_SLOW,  // WiFi STA
    LED_MODE_BT_SPP           = LED_BLINK_FAST,  // BT SPP

    LED_MODE_NULL,
} led_mode_t;


typedef void * blink_led_handle_t;

struct blink_led {
    uint8_t          num;
    led_blink_t      mode;
    uint8_t          state;
#ifdef CONFIG_BLINK_LED_RMT
    led_strip_handle_t strip;
#elif CONFIG_BLINK_LED_GPIO
    //
#endif /* CONFIG_BLINK_LED_RMT CONFIG_BLINK_LED_GPIO */
};

blink_led_handle_t module_led_start(uint8_t num);
void led_mode_set(struct blink_led *led, led_blink_t mode);
void led_mode_next(struct blink_led *led);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __BLINK_SOURCE_H__

