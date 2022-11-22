
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_wifi_types.h"
#include "esp_spp_api.h"
#include "esp_wifi.h"
#include "esp_event_base.h"

#include "util.h"
#include "module.h"
#include "define.h"
#include "process.h"
#include "mode.h"
#include "handle.h"

#define CONFIG_KEY_RESERVE_TIME_IN_MS      (CONFIG_KEY_PRESS_RESERVE*1000)

/* Event source related definitions */
ESP_EVENT_DEFINE_BASE(EVT_PROCESS);

static esp_event_loop_handle_t loop_with_task;
static TaskHandle_t application_task_hdl;

static int64_t press_act_time                   = 0;
static int64_t press_rel_time                   = 0;

static void task_evt_process(void* args)
{
    // Wait to be started by the main task
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    while(1) {
        //ESP_LOGI(MODULE_EVT_PROC, "application_task: running application task");
        //esp_event_loop_run(loop_with_task, 100);
        vTaskDelay(10);
    }
}

static void evt_process_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    static uint16_t next_mode = SW_MODE_WIFI_AP;
    int64_t curr_time;
    curr_time = esp_timer_get_time();
    
    switch(id){
        case MODE_KEY_SHORT_PRESSED:
            if(TIME_DIFF_IN_MS(press_rel_time, curr_time) > CONFIG_KEY_RESERVE_TIME_IN_MS){
                next_mode = snap_sw_mode_next();
            }
            if (TIME_DIFF_IN_MS(press_act_time, curr_time) < CONFIG_KEY_RESERVE_TIME_IN_MS){
                next_mode++;
                ESP_LOGI(MODULE_EVT_PROC, "act mode=%d,  %lld < %d in ms", 
                        next_mode, TIME_DIFF_IN_MS(press_act_time, curr_time), CONFIG_KEY_RESERVE_TIME_IN_MS);
                return;
            }
            press_act_time = curr_time;
            
#if (DEBUG_EVT_PROC)
            ESP_LOGI(MODULE_EVT_PROC, "Switch from %d--------", snap_sw_mode_get());
#endif /* DEBUG_EVT_PROC */
            
            esp_err_t ret = snap_sw_mode_switch(next_mode);

#if (DEBUG_EVT_PROC)
            ESP_LOGI(MODULE_EVT_PROC, "Switch to %d-------- ret = %d", snap_sw_mode_get(), ret);
#else
            UNUSED(ret)
#endif /*DEBUG_EVT_PROC*/

            led_mode_next((struct blink_led *)g_led_handle);

            break;

        case MODE_KEY_RELEASED:
            press_rel_time = curr_time;
            ESP_LOGW(MODULE_EVT_PROC, "To be implemented");
            
            break;

        case MODE_KEY_LONG_PRESSED:
            snap_reboot(3);
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
        .queue_size = 5,
        .task_name = "loop_task", // task will be created
        .task_priority = uxTaskPriorityGet(NULL),
        .task_stack_size = 3072,
        .task_core_id = tskNO_AFFINITY
    };

    // Create the event loops
    ESP_ERROR_CHECK(esp_event_loop_create(&loop_with_task_args, &loop_with_task));

    // Register the handler for task iteration event. Notice that the same handler is used for handling event on different loops.
    // The loop handle is provided as an argument in order for this example to display the loop the handler is being run on.
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(loop_with_task, EVT_PROCESS, ESP_EVENT_ANY_ID, evt_process_handler, loop_with_task, NULL));

    // Create the application task
    xTaskCreate(task_evt_process, MODULE_EVT_PROC, 3072, NULL, uxTaskPriorityGet(NULL) + 1, &application_task_hdl);

    // Start the application task to run the event handlers
    xTaskNotifyGive(application_task_hdl);

    return loop_with_task;
}


