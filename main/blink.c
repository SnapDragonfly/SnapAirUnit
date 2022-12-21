/// @file blink.c

/*
 * idf header files
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

/*
 * basic header files
 */
#include "module.h"
#include "define.h"

/*
 * module header files
 */
//TBD

/*
 * service header files
 */
#include "blink.h"

static void led_init(struct blink_led *led, uint8_t io_num)
{
    led->num      = io_num;
    led->mode     = LED_BLINK_NONE;
    led->state    = 1;
}

void led_mode_set(struct blink_led *led, led_mode_t mode)
{
    led->mode     = mode%LED_BLINK_NULL;
    led->state    = 1;
}

void led_mode_next(struct blink_led *led)
{
    led->mode++;
    led->mode     = led->mode % LED_BLINK_NULL;
    led->state    = 1;
}


#ifdef CONFIG_BLINK_LED_RMT

static void led_set(struct blink_led *led)
{
    /* If the addressable LED is enabled */
    if (led->state) {
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
        led_strip_set_pixel(led->strip, 0, 16, 16, 16);
        /* Refresh the strip to send data */
        led_strip_refresh(led->strip);
    } else {
        /* Set all LED off to clear all pixels */
        led_strip_clear(led->strip);
    }
}

static void led_configure(struct blink_led *led)
{
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = led->num,
        .max_leds = 1, // at least one LED on board
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led->strip));
    /* Set all LED off to clear all pixels */
    led_strip_clear(led->strip);
}

#elif CONFIG_BLINK_LED_GPIO

static void led_set(struct blink_led *led)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(led->num, led->state);
}

static void led_configure(struct blink_led *led)
{
    gpio_reset_pin(led->num);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(led->num, GPIO_MODE_OUTPUT);
}

#endif /* CONFIG_BLINK_LED_RMT CONFIG_BLINK_LED_GPIO */

static void task_led_blink(void* args)
{
    struct blink_led *led = (struct blink_led *)args;

    while (1) {
        switch(led->mode){
            case LED_BLINK_NONE:
#if (DEBUG_LED_BLINK)
                ESP_LOGI(MODULE_LED_BLINK, "Turning the LED %s!", led->state == true ? "ON" : "OFF");
#endif /* DEBUG_LED_BLINK */
                
                led_set(led);  //Check active IO level of led on

                vTaskDelay(CONFIG_BLINK_INTERVAL / portTICK_PERIOD_MS);
                break;

            case LED_BLINK_SLOW:
#if (DEBUG_LED_BLINK)
                ESP_LOGI(MODULE_LED_BLINK, "Turning the LED %s!", led->state == true ? "ON" : "OFF");
#endif /* DEBUG_LED_BLINK */
                
                led_set(led);  //Check active IO level of led on

                /* Toggle the LED state */
                led->state = !led->state;

                vTaskDelay(CONFIG_BLINK_PERIOD_SLOW / portTICK_PERIOD_MS);
                
                break;

            case LED_BLINK_FAST:
#if (DEBUG_LED_BLINK)
                ESP_LOGI(MODULE_LED_BLINK, "Turning the LED %s!", led->state == true ? "ON" : "OFF");
#endif /* DEBUG_LED_BLINK */
                
                led_set(led);  //Check active IO level of led on

                /* Toggle the LED state */
                led->state = !led->state;

                vTaskDelay(CONFIG_BLINK_PERIOD_FAST / portTICK_PERIOD_MS);
                
                break;

            case LED_BLINK_WARN:
                for(int i = 0; i < 2*CONFIG_BLINK_SUCCESSIVE_WARNING; i++){
#if (DEBUG_LED_BLINK)
                    ESP_LOGI(MODULE_LED_BLINK, "Turning the LED %s!", led->state == true ? "ON" : "OFF");
#endif /* DEBUG_LED_BLINK */

                    led_set(led);  //Check active IO level of led on

                    /* Toggle the LED state */
                    led->state = !led->state;

                    vTaskDelay(CONFIG_BLINK_PERIOD_FAST / portTICK_PERIOD_MS);
                }

                vTaskDelay(CONFIG_BLINK_INTERVAL / portTICK_PERIOD_MS);
                
                break;

            case LED_BLINK_ERROR:
                for(int i = 0; i < 2*CONFIG_BLINK_SUCCESSIVE_ERROR; i++){
#if (DEBUG_LED_BLINK)
                    ESP_LOGI(MODULE_LED_BLINK, "Turning the LED %s!", led->state == true ? "ON" : "OFF");
#endif /* DEBUG_LED_BLINK */
                    
                    led_set(led);  //Check active IO level of led on

                    /* Toggle the LED state */
                    led->state = !led->state;

                    vTaskDelay(CONFIG_BLINK_PERIOD_FAST / portTICK_PERIOD_MS);
                }

                vTaskDelay(CONFIG_BLINK_INTERVAL / portTICK_PERIOD_MS);

                break;

            case LED_BLINK_NULL:
            default:
                /* Can't be HERE */
                ESP_LOGE(MODULE_LED_BLINK, "Error");
                break;
            }
    }
}

blink_led_handle_t module_led_start(uint8_t num)
{
    struct blink_led *pled = malloc(sizeof(struct blink_led));

    led_init(pled, num);
    led_configure(pled);

    xTaskCreate(task_led_blink, MODULE_LED_BLINK, TASK_SIMPLE_BUFFER, pled, uxTaskPriorityGet(NULL), NULL);
    ESP_LOGI(MODULE_LED_BLINK, "OK %d", pled->mode);

    return pled;
}

