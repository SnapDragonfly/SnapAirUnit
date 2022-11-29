/* SPIFFS filesystem example.
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#include "define.h"
#include "module.h"
#include "spiffs.h"

static esp_vfs_spiffs_conf_t g_spiffs_conf = {
  .base_path = CONFIG_SPIFFS_MOUNT_POINT,
  .partition_label = "storage",
  .max_files = 5,
  .format_if_mount_failed = true
};

#define TEST_FILE1 "/hello.txt"
#define TEST_FILE2 "/foo.txt"

esp_err_t stop_spiffs(void)
{
    // All done, unmount partition and disable SPIFFS
    esp_vfs_spiffs_unregister(g_spiffs_conf.partition_label);
    ESP_LOGI(MODULE_SPIFFS, "SPIFFS unmounted");

    return ESP_OK;
}

esp_err_t spiffs_test(void)
{
    esp_err_t ret;

    char str_buf1[64];
    char str_buf2[64];

    // Use POSIX and C standard library functions to work with files.
    // First create a file.
    ESP_LOGI(MODULE_SPIFFS, "Opening file");
    sprintf(str_buf1, "%s%s", CONFIG_SPIFFS_MOUNT_POINT, TEST_FILE1);
    FILE* f = fopen(str_buf1, "w");
    if (f == NULL) {
        ESP_LOGE(MODULE_SPIFFS, "Failed to open file for writing");
        return ret;
    }
    fprintf(f, "Hello World!\n");
    fclose(f);
    ESP_LOGI(MODULE_SPIFFS, "File written");

    // Check if destination file exists before renaming
    struct stat st;
    sprintf(str_buf2, "%s%s", CONFIG_SPIFFS_MOUNT_POINT, TEST_FILE2);
    if (stat(str_buf2, &st) == 0) {
        // Delete it if it exists
        unlink(str_buf2);
    }

    // Rename original file
    ESP_LOGI(MODULE_SPIFFS, "Renaming file");
    sprintf(str_buf1, "%s%s", CONFIG_SPIFFS_MOUNT_POINT, TEST_FILE1);
    sprintf(str_buf2, "%s%s", CONFIG_SPIFFS_MOUNT_POINT, TEST_FILE2);
    if (rename(str_buf1, str_buf2) != 0) {
        ESP_LOGE(MODULE_SPIFFS, "Rename failed");
        return ret;
    }

    // Open renamed file for reading
    ESP_LOGI(MODULE_SPIFFS, "Reading file");
    sprintf(str_buf2, "%s%s", CONFIG_SPIFFS_MOUNT_POINT, TEST_FILE2);
    f = fopen(str_buf2, "r");
    if (f == NULL) {
        ESP_LOGE(MODULE_SPIFFS, "Failed to open file for reading");
        return ret;
    }
    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);
    // strip newline
    char* pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(MODULE_SPIFFS, "Read from file: '%s'", line);

    return ESP_OK;
}


esp_err_t start_spiffs(void)

{
    ESP_LOGI(MODULE_SPIFFS, "Initializing SPIFFS");


    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&g_spiffs_conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(MODULE_SPIFFS, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(MODULE_SPIFFS, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(MODULE_SPIFFS, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

#ifdef CONFIG_EXAMPLE_SPIFFS_CHECK_ON_START
    ESP_LOGI(MODULE_SPIFFS, "Performing SPIFFS_check().");
    ret = esp_spiffs_check(g_spiffs_conf.partition_label);
    if (ret != ESP_OK) {
        ESP_LOGE(MODULE_SPIFFS, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
        return ret;
    } else {
        ESP_LOGI(MODULE_SPIFFS, "SPIFFS_check() successful");
    }
#endif

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(g_spiffs_conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(MODULE_SPIFFS, "Failed to get SPIFFS partition information (%s). Formatting...", esp_err_to_name(ret));
        esp_spiffs_format(g_spiffs_conf.partition_label);
        return ret;
    } else {
        ESP_LOGI(MODULE_SPIFFS, "Partition size: total: %d, used: %d", total, used);
    }

    // Check consistency of reported partiton size info.
    if (used > total) {
        ESP_LOGW(MODULE_SPIFFS, "Number of used bytes cannot be larger than total. Performing SPIFFS_check().");
        ret = esp_spiffs_check(g_spiffs_conf.partition_label);
        // Could be also used to mend broken files, to clean unreferenced pages, etc.
        // More info at https://github.com/pellepl/spiffs/wiki/FAQ#powerlosses-contd-when-should-i-run-spiffs_check
        if (ret != ESP_OK) {
            ESP_LOGE(MODULE_SPIFFS, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
            return ret;
        } else {
            ESP_LOGI(MODULE_SPIFFS, "SPIFFS_check() successful");
        }
    }

    return ESP_OK;
}
