/// @file util.c

/*
 * idf header files
 */
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "time.h"
#include "sys/time.h"
#include "sdkconfig.h"

/*
 * basic header files
 */
#include "config.h"
#include "define.h"

/*
 * module header files
 */
#include "util.h"

/*
 * service header files
 */
//TBD

void UTIL_reboot(int in_seconds)
{
    for (int i = in_seconds; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(TIME_ONE_SECOND_IN_MS / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}

void UTIL_alive(char c)
{
    while(1) {
        putchar(c);
        fflush(stdout);
        vTaskDelay(TIME_ONE_SECOND_IN_MS / portTICK_PERIOD_MS);
    }
}

void UTIL_idle(void)
{
    while(1) {
        vTaskDelay(TIME_ONE_SECOND_IN_MS / portTICK_PERIOD_MS);
    }
}


