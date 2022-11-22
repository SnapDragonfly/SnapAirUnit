
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_check.h"
#include "esp_event.h"

#include "ttl.h"
#include "mode.h"
#include "module.h"
#include "define.h"
#include "key.h"

esp_err_t snap_sw_module_start(TaskFunction_t pxTaskCode, bool task, const uint32_t stackDepth, const char * const pcName)
{
    if(!pxTaskCode){
        return ESP_FAIL;
    }

    /* 
     * Prevent multi key interrupts
     * ToDo:
     * a) more robust module frame work should be considered.
     * b) esp examples (AP/STA/BT SPP) should be insight learned, 
     *    especially how to judge service can be destroied.
     */
    mode_key_lock();
    
    if (task){
        xTaskCreate(pxTaskCode, pcName, stackDepth, NULL, uxTaskPriorityGet(NULL), NULL);
    }else{
        pxTaskCode(NULL);
    }

    mode_key_unlock();

    return ESP_OK;
}


