/// @file factory_setting.c

/*
 * idf header files
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_netif_ip_addr.h"
#include "esp_err.h"
#include "nvs.h"

/*
 * basic header files
 */
#include "config.h"
#include "define.h"

/*
 * module header files
 */
#include "module.h"
#include "factory_setting.h"
#include "cmd_nvs.h"

/*
 * service header files
 */
//TBD

static char g_str_ap_ssid[WIFI_SSID_LENGTH];
static char g_str_ap_pass[WIFI_PASS_LENGTH];

static char g_str_sta_ssid[WIFI_SSID_LENGTH];
static char g_str_sta_pass[WIFI_PASS_LENGTH];

static char g_str_ip[STR_IP_LEN];

esp_err_t restore_ap_settings(void)
{
    esp_err_t err;

    memset(g_str_ap_ssid, 0 , WIFI_SSID_LENGTH);
    strncpy(g_str_ap_ssid, FACTORY_ESP_WIFI_AP_SSID, WIFI_SSID_LENGTH);
    memset(g_str_ap_pass, 0 , WIFI_PASS_LENGTH);
    strncpy(g_str_ap_pass, FACTORY_ESP_WIFI_AP_PASS, WIFI_PASS_LENGTH);

#if (DEBUG_FACTORY_SETTING)
    ESP_LOGI(MODULE_FACTORY_SETTING, "Factory ap setting used: %s, %s", g_str_ap_ssid, g_str_ap_pass);
#endif /* DEBUG_FACTORY_SETTING */

    err = nvs_set_wifi_ap(g_str_ap_ssid, g_str_ap_pass);
    if (err != ESP_OK){
        ESP_LOGW(MODULE_FACTORY_SETTING, "Save AP ssid & pass failed!");
    } else {
        ESP_LOGI(MODULE_FACTORY_SETTING, "Restore AP ssid & pass OK!");
    }

    return err;
}

esp_err_t restore_sta_settings(void)
{
    esp_err_t err;

    memset(g_str_sta_ssid, 0 , WIFI_SSID_LENGTH);
    strncpy(g_str_sta_ssid, FACTORY_ESP_WIFI_STA_SSID, WIFI_SSID_LENGTH);
    memset(g_str_sta_pass, 0 , WIFI_PASS_LENGTH);
    strncpy(g_str_sta_pass, FACTORY_ESP_WIFI_STA_PASS, WIFI_PASS_LENGTH);

#if (DEBUG_FACTORY_SETTING)
    ESP_LOGI(MODULE_FACTORY_SETTING, "Factory sta setting used: %s, %s", g_str_sta_ssid, g_str_sta_pass);
#endif /* DEBUG_FACTORY_SETTING */

    err = nvs_set_wifi_sta(g_str_sta_ssid, g_str_sta_pass);
    if (err != ESP_OK){
        ESP_LOGW(MODULE_FACTORY_SETTING, "Save Station ssid & pass failed!");
    } else {
        ESP_LOGI(MODULE_FACTORY_SETTING, "Restore Station ssid & pass OK!");
    }

    return err;
}

void factory_settings_init(void* args)
{
    esp_err_t err;

    err = nvs_get_wifi_ap(g_str_ap_ssid, WIFI_SSID_LENGTH, g_str_ap_pass, WIFI_PASS_LENGTH);
    if (err != ESP_OK){
        ESP_LOGW(MODULE_FACTORY_SETTING, "AP parameters read err, use default %s, %s", g_str_ap_ssid, g_str_ap_pass);
        (void)restore_ap_settings();
    } else {
#if (DEBUG_FACTORY_SETTING)
        ESP_LOGI(MODULE_FACTORY_SETTING, "AP parameters read: %s, %s", g_str_ap_ssid, g_str_ap_pass);
#endif /* DEBUG_FACTORY_SETTING */
        
    }

    err = nvs_get_wifi_sta(g_str_sta_ssid, WIFI_SSID_LENGTH, g_str_sta_pass, WIFI_PASS_LENGTH);
    if (err != ESP_OK){
        ESP_LOGW(MODULE_FACTORY_SETTING, "STA parameters read err, use default  %s, %s", g_str_sta_ssid, g_str_sta_pass);
        (void)restore_sta_settings();
    } else {
#if (DEBUG_FACTORY_SETTING)
        ESP_LOGI(MODULE_FACTORY_SETTING, "STA parameters read: %s, %s", g_str_sta_ssid, g_str_sta_pass);
#endif /* DEBUG_FACTORY_SETTING */
    }
}


esp_err_t restore_factory_settings(void)
{
    esp_err_t err1, err2;

    err1 = restore_ap_settings();

#if (DEBUG_FACTORY_SETTING)
    if (err1 != ESP_OK){
        ESP_LOGW(MODULE_FACTORY_SETTING, "Save AP ssid & pass failed!");
    } else {
        ESP_LOGI(MODULE_FACTORY_SETTING, "Restore AP ssid & pass OK!");
    }
#endif /* DEBUG_FACTORY_SETTING */

    err2 = restore_sta_settings();

#if (DEBUG_FACTORY_SETTING)
    if (err2 != ESP_OK){
        ESP_LOGW(MODULE_FACTORY_SETTING, "Save STA ssid & pass failed!");
    } else {
        ESP_LOGI(MODULE_FACTORY_SETTING, "Restore STA ssid & pass OK!");
    }
#endif /* DEBUG_FACTORY_SETTING */

    return err1 != ESP_OK? err1 : err2;
}



esp_err_t set_ap_settings(const char * ssid, const char * pass)
{
    esp_err_t err;
    ssize_t ssid_len, pass_len; 
    if (NULL == ssid || NULL == pass){
        return ESP_FAIL;
    }

    ssid_len = strlen(ssid);
    pass_len = strlen(pass);
    if(ssid_len > (WIFI_SSID_LENGTH-1) || pass_len > (WIFI_PASS_LENGTH-1)){
        return ESP_FAIL;
    }

    memset(g_str_ap_ssid, 0 , WIFI_SSID_LENGTH);
    strncpy(g_str_ap_ssid, ssid, ssid_len);
    memset(g_str_ap_pass, 0 , WIFI_PASS_LENGTH);
    strncpy(g_str_ap_pass, pass, pass_len);

#if (DEBUG_FACTORY_SETTING)
    ESP_LOGI(MODULE_FACTORY_SETTING, "ap setting used: %s, %s", g_str_ap_ssid, g_str_ap_pass);
#endif /* DEBUG_FACTORY_SETTING */

    err = nvs_set_wifi_ap(g_str_ap_ssid, g_str_ap_pass);
    if (err != ESP_OK){
        ESP_LOGW(MODULE_FACTORY_SETTING, "Save AP ssid & pass failed!");
    } else {
        ESP_LOGI(MODULE_FACTORY_SETTING, "Save AP ssid & pass OK!");
    }

    return err;
}

esp_err_t set_sta_settings(const char * ssid, const char * pass)
{
    esp_err_t err;
    ssize_t ssid_len, pass_len; 
    if (NULL == ssid || NULL == pass){
        return ESP_FAIL;
    }

    ssid_len = strlen(ssid);
    pass_len = strlen(pass);
    if(ssid_len > (WIFI_SSID_LENGTH-1) || pass_len > (WIFI_PASS_LENGTH-1)){
        return ESP_FAIL;
    }

    memset(g_str_sta_ssid, 0 , WIFI_SSID_LENGTH);
    strncpy(g_str_sta_ssid, ssid, ssid_len);
    memset(g_str_sta_pass, 0 , WIFI_PASS_LENGTH);
    strncpy(g_str_sta_pass, pass, pass_len);

#if (DEBUG_FACTORY_SETTING)
    ESP_LOGI(MODULE_FACTORY_SETTING, "ap setting used: %s, %s", g_str_sta_ssid, g_str_sta_pass);
#endif /* DEBUG_FACTORY_SETTING */

    err = nvs_set_wifi_sta(g_str_sta_ssid, g_str_sta_pass);
    if (err != ESP_OK){
        ESP_LOGW(MODULE_FACTORY_SETTING, "Save AP ssid & pass failed!");
    } else {
        ESP_LOGI(MODULE_FACTORY_SETTING, "Save AP ssid & pass OK!");
    }

    return err;
}

esp_err_t set_str_ip(uint16_t a, uint16_t b, uint16_t c, uint16_t d)
{
    memset(g_str_ip, 0 , STR_IP_LEN);
    snprintf(g_str_ip, STR_IP_LEN, IPSTR, a, b, c, d);

    return ESP_OK;
}


char * get_ap_ssid(void)
{
    return &g_str_ap_ssid[0];
}

char * get_ap_pass(void)
{
    return &g_str_ap_pass[0];
}

char * get_sta_ssid(void)
{
    return &g_str_sta_ssid[0];
}
char * get_sta_pass(void)
{
    return &g_str_sta_pass[0];
}

char * get_idf_versions(void)
{
    return IDF_VER;
}

char * get_app_versions(void)
{
    return APP_VERSION;
}

char * get_str_ip(void)
{
    return &g_str_ip[0];
}

