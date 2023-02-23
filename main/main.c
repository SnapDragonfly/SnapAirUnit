/// @file main.c

/*
 * idf header files
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_task_wdt.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

/*
 * basic header files
 */
#include "config.h"
#include "define.h"
#include "handle.h"

/*
 * module header files
 */
#include "module.h"
#include "msp_protocol.h"
#include "tello_protocol.h"
#include "util.h"
#include "mode.h"
#include "process.h"
#include "spiffs.h"
#include "console.h"
#include "factory_setting.h"

/*
 * service header files
 */
#include "softap.h"
#include "station.h"
#include "btspp.h"
#include "blink.h"
#include "key.h"
#include "ttl.h"
#include "udp_server.h"
#include "udp_client.h"
#include "rest_server.h"

/**
 * This is experimental code of the application.
 * 
*/
void sand_box(void)
{
    /* 
     * Snap Air Unit: Test Codes HERE
     * Remove this asap when you commit codes 
     */

    // ...
    //spiffs_test();
    //spiffs_stop();


    /*
     * Console for back door debug
     * Which never returns.
     */
    ESP_ERROR_CHECK(module_console_start());

    /* Shouldn't BE HERE!!! */
    (void)UTIL_alive(SAND_BOX_ALIVE_CHARACTER);
    (void)UTIL_reboot(SAND_BOX_REBOOT_PROMOTES);

#if defined(PASS_THROUGH_UART)
    /* Issue #67, need to be fixed.
     * Compiler error when PASS_THROUGH_UART selected (avoided, but NOT solved properly) #67 
     */
    //UNUSED(nomsp_handle_tello);
    nomsp_handle_tello(NULL, 0);
#elif defined(PASS_THROUGH_HY)
    //UNUSED(x);
#else /* PASS_THROUGH_MSP */
    //UNUSED(x);
#endif /* PASS_THROUGH_DATA */

    
    
}

/**
 * This is initialization code of the application.
 * 
*/
void app_init(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }

    //Initialize default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    //Initialize netif stack
    ESP_ERROR_CHECK(esp_netif_init());

    //Initialize consistent storage
    ESP_ERROR_CHECK(spiffs_start());
    ESP_ERROR_CHECK(snap_sw_module_start(factory_settings_init, false, 0, MODULE_FACTORY_SETTING));

    //Basic Hardware & Software Components
    g_evt_handle    = module_evt_start();
    g_mode_handle   = module_led_start(MODE_GPIO);
    g_status_handle = module_led_start(STATUS_GPIO);
    g_key_handle    = module_key_start(KEY_MODE);

    //Service Module for Applications

#if defined(PASS_THROUGH_UART)
    ESP_ERROR_CHECK(snap_sw_module_start(rest_srv_init, false, 0, MODULE_HTTP));
    ESP_ERROR_CHECK(ttl_srv_start());
    ESP_ERROR_CHECK(udp_srv_start());
    ESP_ERROR_CHECK(wireless_mode_init());
#elif defined(PASS_THROUGH_HY)
    ESP_ERROR_CHECK(snap_sw_module_start(rest_srv_init, false, 0, MODULE_HTTP));
    ESP_ERROR_CHECK(ttl_srv_start());
    ESP_ERROR_CHECK(udp_srv_start());
    ESP_ERROR_CHECK(udp_clt_start());
    ESP_ERROR_CHECK(wireless_mode_init());
#else /* PASS_THROUGH_MSP */
    ESP_ERROR_CHECK(snap_sw_module_start(rest_srv_init, false, 0, MODULE_HTTP));
    ESP_ERROR_CHECK(ttl_srv_start());
    ESP_ERROR_CHECK(udp_srv_start());
    ESP_ERROR_CHECK(udp_clt_start());
    ESP_ERROR_CHECK(wireless_mode_init());
    ESP_ERROR_CHECK(auc_srv_start());
#endif /* PASS_THROUGH_DATA */

    printf("%s: free_heap_size = %d\n", DEVICE_NAME_SNAP_AIR_UNIT, esp_get_free_heap_size());
}


/**
 * This is the entry function of ESP32.
 * 
*/
void app_main(void)
{
    /* 
     * Snap Air Unit: Education Version 
     */
#if defined(PASS_THROUGH_UART)
    printf("%s: Warmly welcome! udp-uart mode\n", DEVICE_NAME_SNAP_AIR_UNIT);
#elif defined(PASS_THROUGH_HY)
    printf("%s: Warmly welcome! udp-hy mode\n", DEVICE_NAME_SNAP_AIR_UNIT);
#else /* PASS_THROUGH_MSP */
    printf("%s: Warmly welcome! udp-msp mode\n", DEVICE_NAME_SNAP_AIR_UNIT);
#endif /* PASS_THROUGH_DATA */

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
    printf("Current free heap size: %d bytes\n", esp_get_free_heap_size());
    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

    /* Snap Air Unit: Modules & Components */
    (void)app_init();

    /* Snap Air Unit: Sandbox */
    (void)sand_box();
}


