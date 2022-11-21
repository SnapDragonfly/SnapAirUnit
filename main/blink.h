
#ifndef __BLINK_SOURCE_H__
#define __BLINK_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define BLINK_GPIO CONFIG_BLINK_GPIO

typedef enum {
    LED_BLINK_NONE            = 0,
    LED_BLINK_SLOW            = 1,
    LED_BLINK_FAST            = 2,
    LED_BLINK_WARN            = 3,
    LED_BLINK_ERROR           = 4,

    LED_BLINK_NULL,
} led_mode_t;

typedef void * blink_led_handle_t;


struct blink_led {
    uint8_t    num;
    led_mode_t mode;
    uint8_t    state;
#ifdef CONFIG_BLINK_LED_RMT
    led_strip_handle_t strip;
#elif CONFIG_BLINK_LED_GPIO
    //
#endif /* CONFIG_BLINK_LED_RMT CONFIG_BLINK_LED_GPIO */
};

blink_led_handle_t module_led_start(uint8_t num);
void led_mode_set(struct blink_led *led, led_mode_t mode);
void led_mode_next(struct blink_led *led);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __BLINK_SOURCE_H__

