
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "nvs_flash.h"

#include "util.h"
#include "softap.h"
#include "station.h"
#include "btspp.h"
#include "mode.h"

#include "module.h"
#include "define.h"
#include "blink.h"
#include "key.h"
#include "ttl.h"
#include "process.h"
#include "rest_server.h"
#include "udp_server.h"
#include "udp_client.h"
#include "spiffs.h"
#include "console.h"

void sand_box(void)
{
    /* 
     * Snap Air Unit: Test Codes HERE
     * Remove this asap when you commit codes 
     */

    // ...
    //spiffs_test();
    //stop_spiffs();


    // Console for back dorr debug
    ESP_ERROR_CHECK(module_console_start());

    snap_alive(SAND_BOX_ALIVE_CHARACTER);
    snap_reboot(SAND_BOX_REBOOT_PROMOTES);

}

/* 
 * Snap Air Unit: Main
 */

blink_led_handle_t             g_led_handle = NULL;
button_handle_t                g_key_handle = NULL;
esp_event_loop_handle_t        g_evt_handle = NULL;

void app_init(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    //Basic Software Components
    g_evt_handle = module_evt_start();
    ESP_ERROR_CHECK(start_spiffs());
    (void)start_factory_settings();

    //Basic Hardware Components
    g_led_handle = module_led_start(BLINK_GPIO);
    g_key_handle = module_key_start(KEY_MODE);
    ESP_ERROR_CHECK(module_ttl_start());


    //Service Module for Applications
    snap_sw_mode_init();
    sanp_sw_rest_init();
    ESP_ERROR_CHECK(start_udp_server());
    ESP_ERROR_CHECK(start_udp_client());
    wifi_init_sta();

    printf("%s: free_heap_size = %d\n", DEVICE_NAME_SNAP_AIR_UNIT, esp_get_free_heap_size());
}

void app_main(void)
{
    /* 
     * Snap Air Unit: Education Version 
     */
    printf("%s: Warmly welcome!\n", DEVICE_NAME_SNAP_AIR_UNIT);

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%uMB %s flash\n", flash_size / (1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

    /* Snap Air Unit: Modules & Components */
    app_init();

    /* Snap Air Unit: Sandbox */
    sand_box();
}


