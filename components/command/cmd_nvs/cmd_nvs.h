/* Console example — declarations of command registration functions.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Register NVS functions
void register_nvs(void);

// for nvs storage
esp_err_t nvs_get_wifi_ap(char * ssid, size_t ssid_len, char * pass, size_t pass_len);
esp_err_t nvs_get_wifi_sta(char * ssid, size_t ssid_len, char * pass, size_t pass_len);
esp_err_t nvs_get_wireless_mode(uint16_t* mode);
esp_err_t nvs_set_wifi_ap(char * ssid, char * pass);
esp_err_t nvs_set_wifi_sta(char * ssid, char * pass);
esp_err_t nvs_set_wireless_mode(uint16_t mode);


#ifdef __cplusplus
}
#endif
