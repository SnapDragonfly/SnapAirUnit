/// @file process.c

/*
 * idf header files
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_wifi_types.h"
#include "esp_spp_api.h"
#include "esp_wifi.h"
#include "esp_event_base.h"

/*
 * basic header files
 */
#include "config.h"
#include "define.h"
#include "handle.h"

/*
 * module header files
 */
#include "util.h"
#include "module.h"
#include "process.h"
#include "mode.h"
#include "factory_setting.h"

/*
 * service header files
 */
//TBD


/* Event source related definitions */
ESP_EVENT_DEFINE_BASE(EVT_PROCESS);

static esp_event_loop_handle_t g_loop_with_task;
static TaskHandle_t          g_application_task_hdl;

static int64_t press_act_time                   = 0;
static int64_t press_rel_time                   = 0;

static void evt_process_task(void* args)
{
    UNUSED(args);

    // Wait to be started by the main task
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    while(1) {

        /*
         * Monitor WiFi Station Connect Failure
         * Fall back to WiFi AP mode by default
         */
        extern bool g_wifi_sta_start;
        if(!g_wifi_sta_start && SW_MODE_NULL == wireless_mode_get()){
            wireless_mode_set(SW_MODE_WIFI_STA);

            ESP_ERROR_CHECK(esp_event_post_to(g_evt_handle, EVT_PROCESS, MODE_KEY_DEFAULT, NULL, 0, portMAX_DELAY));
            //ESP_ERROR_CHECK(esp_event_post(EVT_PROCESS, MODE_KEY_DEFAULT, NULL, 0, portMAX_DELAY));
            ESP_LOGI(MODULE_CONSOLE, "Trigger MODE_KEY_DEFAULT\n");
        }

        vTaskDelay(10);
    }
}

static void evt_process_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    esp_err_t err;
    
    static uint16_t next_mode = SW_MODE_WIFI_AP;
    int64_t curr_time        = esp_timer_get_time();

#if (DEBUG_EVT_PROC)
    ESP_LOGI(MODULE_EVT_PROC, "evt_process_handler id %d data 0x%x", id, event_data);
#endif /* DEBUG_EVT_PROC */

    switch(id){
        case MODE_KEY_SHORT_PRESSED:
            if(NULL == event_data ){

                // amount of time from key released
                if(TIME_DIFF_IN_MS(press_rel_time, curr_time) > CONFIG_KEY_RESERVE_TIME_IN_MS){
                    next_mode = wireless_mode_next();
                }

                // amount of time from lastkey press action
                if (TIME_DIFF_IN_MS(press_act_time, curr_time) < CONFIG_KEY_RESERVE_TIME_IN_MS){
                    next_mode++;
                    ESP_LOGI(MODULE_EVT_PROC, "MODE_KEY_SHORT_PRESSED, act mode %d %lld < %d in ms", 
                            next_mode, TIME_DIFF_IN_MS(press_act_time, curr_time), CONFIG_KEY_RESERVE_TIME_IN_MS);
                    return;
                }
                press_act_time = curr_time;

                enum_wireless_mode_t prev_mode = wireless_mode_get();
                err = wireless_mode_switch(next_mode);

#if (DEBUG_EVT_PROC)
                ESP_LOGI(MODULE_EVT_PROC, "MODE_KEY_SHORT_PRESSED, switch from %d to %d err = %d", prev_mode, wireless_mode_get(), err);
#else
                UNUSED(prev_mode);
                UNUSED(err);
#endif /*DEBUG_EVT_PROC*/

            }else{

                int mode = *((int *)event_data);
                err = wireless_mode_switch(mode);

#if (DEBUG_EVT_PROC)
                ESP_LOGI(MODULE_EVT_PROC, "MODE_KEY_SHORT_PRESSED, switch to mode %d err = %d", wireless_mode_get(), err);
#else
                UNUSED(err);
#endif /*DEBUG_EVT_PROC*/

            }

            break;

        case MODE_KEY_RELEASED:
            press_rel_time = curr_time;
            ESP_LOGW(MODULE_EVT_PROC, "MODE_KEY_RELEASED, To be implemented");

            break;

        case MODE_KEY_LONG_PRESSED:
            (void)restore_factory_settings();
            (void)UTIL_reboot(3);
            break;

        case MODE_KEY_DEFAULT:
            err = wireless_mode_switch(SW_MODE_WIFI_AP);
#if (DEBUG_EVT_PROC)
            ESP_LOGI(MODULE_EVT_PROC, "MODE_KEY_DEFAULT, switch to mode %d err = %d", wireless_mode_get(), err);
#endif /* DEBUG_EVT_PROC */

            break;
            
        default:
            /* Can't be HERE */
            ESP_LOGE(MODULE_EVT_PROC, "Error");

            break;
        }
}

esp_event_loop_handle_t module_evt_start(void)
{
#if (DEBUG_EVT_PROC)
    ESP_LOGI(MODULE_EVT_PROC, "setting up");
#endif /* DEBUG_EVT_PROC */

    esp_event_loop_args_t loop_with_task_args = {
        .queue_size = EVT_QUEUE_SIZE,
        .task_name = MODULE_EVT_PROC, // task will be created
        .task_priority = uxTaskPriorityGet(NULL),
        .task_stack_size = EVT_STACK_SIZE,
        .task_core_id = tskNO_AFFINITY
    };

    // Create the event loops
    ESP_ERROR_CHECK(esp_event_loop_create(&loop_with_task_args, &g_loop_with_task));

    // Register the handler for task iteration event. Notice that the same handler is used for handling event on different loops.
    // The loop handle is provided as an argument in order for this example to display the loop the handler is being run on.
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(g_loop_with_task, EVT_PROCESS, ESP_EVENT_ANY_ID, evt_process_handler, g_loop_with_task, NULL));

    // Create the application task
    xTaskCreate(evt_process_task, MODULE_EVT_PROC, TASK_BUFFER_3K0, NULL, uxTaskPriorityGet(NULL) + 1, &g_application_task_hdl);

    // Start the application task to run the event handlers
    xTaskNotifyGive(g_application_task_hdl);

    return g_loop_with_task;
}


