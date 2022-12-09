
#include <string.h>
#include <fcntl.h>
#include "esp_vfs_semihost.h"
#include "esp_vfs_fat.h"
#include "esp_spiffs.h"
#include "esp_http_server.h"
#include "esp_chip_info.h"
#include "esp_random.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_vfs.h"
#include "cJSON.h"
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "sdmmc_cmd.h"
#include "nvs_flash.h"
#include "mdns.h"
#include "lwip/apps/netbiosns.h"

#include "msp_protocol.h"
#include "protocol_examples_common.h"
#include "define.h"
#include "module.h"

#if CONFIG_RESTFUL_WEB_DEPLOY_SD
#include "driver/sdmmc_host.h"
#endif

#define MDNS_INSTANCE "snap air unit web server"


#define REST_CHECK(a, str, goto_tag, ...)                                              \
    do                                                                                 \
    {                                                                                  \
        if (!(a))                                                                      \
        {                                                                              \
            ESP_LOGE(MODULE_HTTP, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                             \
        }                                                                              \
    } while (0)

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + STR_BUFFER_LEN)
#define SCRATCH_BUFSIZE (1024)

typedef struct rest_server_context {
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath)
{
    const char *type = "text/plain";
    if (CHECK_FILE_EXTENSION(filepath, ".html")) {
        type = "text/html";
    } else if (CHECK_FILE_EXTENSION(filepath, ".js")) {
        type = "application/javascript";
    } else if (CHECK_FILE_EXTENSION(filepath, ".css")) {
        type = "text/css";
    } else if (CHECK_FILE_EXTENSION(filepath, ".png")) {
        type = "image/png";
    } else if (CHECK_FILE_EXTENSION(filepath, ".ico")) {
        type = "image/x-icon";
    } else if (CHECK_FILE_EXTENSION(filepath, ".svg")) {
        type = "text/xml";
    }
    return httpd_resp_set_type(req, type);
}

/* Send HTTP response with the contents of the requested file */
static esp_err_t rest_common_get_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];

    rest_server_context_t *rest_context = (rest_server_context_t *)req->user_ctx;
    strlcpy(filepath, rest_context->base_path, sizeof(filepath));
    if (req->uri[strlen(req->uri) - 1] == '/') {
        strlcat(filepath, "/index.html", sizeof(filepath));
    } else {
        strlcat(filepath, req->uri, sizeof(filepath));
    }
    int fd = open(filepath, O_RDONLY, 0);
    if (fd == -1) {
        ESP_LOGE(MODULE_HTTP, "Failed to open file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    set_content_type_from_file(req, filepath);

    char *chunk = rest_context->scratch;
    ssize_t read_bytes;
    do {
        /* Read file in chunks into the scratch buffer */
        read_bytes = read(fd, chunk, SCRATCH_BUFSIZE);
        if (read_bytes == -1) {
            ESP_LOGE(MODULE_HTTP, "Failed to read file : %s", filepath);
        } else if (read_bytes > 0) {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
                close(fd);
                ESP_LOGE(MODULE_HTTP, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    /* Close file after sending complete */
    close(fd);
    ESP_LOGI(MODULE_HTTP, "File sending complete");
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

/* Simple handler for light brightness control */
static esp_err_t light_brightness_post_handler(httpd_req_t *req)
{
    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;
    if (total_len >= SCRATCH_BUFSIZE) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len) {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    cJSON *root = cJSON_Parse(buf);
    int red = cJSON_GetObjectItem(root, "red")->valueint;
    int green = cJSON_GetObjectItem(root, "green")->valueint;
    int blue = cJSON_GetObjectItem(root, "blue")->valueint;
    ESP_LOGI(MODULE_HTTP, "Light control: red = %d, green = %d, blue = %d", red, green, blue);
    cJSON_Delete(root);
    httpd_resp_sendstr(req, "Post control value successfully");
    return ESP_OK;
}

/* Simple handler for getting system handler */
static esp_err_t system_info_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    cJSON *root = cJSON_CreateObject();
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    cJSON_AddStringToObject(root, "sdk_version", IDF_VER);
    cJSON_AddStringToObject(root, "app_version", APP_VERSION);
    cJSON_AddNumberToObject(root, "model", chip_info.model);
    cJSON_AddNumberToObject(root, "cores", chip_info.cores);
    cJSON_AddNumberToObject(root, "revision", chip_info.revision);
    const char *sys_info = cJSON_Print(root);
    httpd_resp_sendstr(req, sys_info);
    free((void *)sys_info);
    cJSON_Delete(root);
    return ESP_OK;
}

extern uint16_t g_esp_rc_channel[];
static esp_err_t rc_data_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    cJSON *root = cJSON_CreateObject();

    for(int i = 0; i < MAX_SUPPORTED_RC_CHANNEL_COUNT; i++){
    
        char nth_rc_channel[6];
        snprintf(nth_rc_channel, 5, "%d", i);
        nth_rc_channel[5] = 0;
        
        cJSON_AddNumberToObject(root, nth_rc_channel, g_esp_rc_channel[i]);
    }
    const char *rc_data_raw = cJSON_Print(root);
    httpd_resp_sendstr(req, rc_data_raw);
    free((void *)rc_data_raw);
    cJSON_Delete(root);
    return ESP_OK;
}

static httpd_handle_t g_rest_server = NULL;

rest_server_context_t g_rest_context;


/* URI handler for getting web server files */
httpd_uri_t common_get_uri = {
	.uri = "/*",
	.method = HTTP_GET,
	.handler = rest_common_get_handler,
	.user_ctx = &g_rest_context
};

/* URI handler for light brightness control */
httpd_uri_t light_brightness_post_uri = {
	.uri = "/api/v1/light/brightness",
	.method = HTTP_POST,
	.handler = light_brightness_post_handler,
	.user_ctx = &g_rest_context
};

/* URI handler for fetching temperature data */
httpd_uri_t temperature_data_get_uri = {
	.uri = "/api/v1/rc/raw",
	.method = HTTP_GET,
	.handler = rc_data_get_handler,
	.user_ctx = &g_rest_context
};

/* URI handler for fetching system info */
httpd_uri_t system_info_get_uri = {
	.uri = "/api/v1/system/info",
	.method = HTTP_GET,
	.handler = system_info_get_handler,
	.user_ctx = &g_rest_context
};

esp_err_t start_rest_server(const char *base_path)
{
    REST_CHECK(base_path, "wrong base path", err);
    //REST_CHECK(rest_context, "No memory for rest context", err);
    strlcpy(g_rest_context.base_path, base_path, sizeof(g_rest_context.base_path));

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(MODULE_HTTP, "Starting HTTP Server");

    REST_CHECK(httpd_start(&g_rest_server, &config) == ESP_OK, "Start server failed", err);

    httpd_register_uri_handler(g_rest_server, &system_info_get_uri);

    httpd_register_uri_handler(g_rest_server, &temperature_data_get_uri);

    httpd_register_uri_handler(g_rest_server, &light_brightness_post_uri);

    httpd_register_uri_handler(g_rest_server, &common_get_uri);

    return ESP_OK;
err:
    return ESP_FAIL;
}

esp_err_t stop_rest_server(void)
{
    ESP_LOGI(MODULE_HTTP, "Stoping HTTP Server");

    httpd_unregister_uri_handler(g_rest_server, common_get_uri.uri, common_get_uri.method);

    httpd_unregister_uri_handler(g_rest_server, light_brightness_post_uri.uri, light_brightness_post_uri.method);

    httpd_unregister_uri_handler(g_rest_server, temperature_data_get_uri.uri, temperature_data_get_uri.method);

    httpd_unregister_uri_handler(g_rest_server, system_info_get_uri.uri, system_info_get_uri.method);

    ESP_ERROR_CHECK(httpd_stop(g_rest_server));

    return ESP_OK;
}



static void initialise_mdns(void)
{
    mdns_init();
    mdns_hostname_set(CONFIG_RESTFUL_MDNS_HOST_NAME);
    mdns_instance_name_set(MDNS_INSTANCE);

    mdns_txt_item_t serviceTxtData[] = {
        {"board", "esp32"},
        {"path", "/"}
    };

    ESP_ERROR_CHECK(mdns_service_add("Snap-Air-Unit-WebServer", "_http", "_tcp", 80, serviceTxtData,
                                     sizeof(serviceTxtData) / sizeof(serviceTxtData[0])));
}

#if CONFIG_RESTFUL_WEB_DEPLOY_SEMIHOST
esp_err_t init_fs(void)
{
    esp_err_t ret = esp_vfs_semihost_register(CONFIG_RESTFUL_WEB_MOUNT_POINT);
    if (ret != ESP_OK) {
        ESP_LOGE(MODULE_HTTP, "Failed to register semihost driver (%s)!", esp_err_to_name(ret));
        return ESP_FAIL;
    }
    return ESP_OK;
}
#endif

#if CONFIG_RESTFUL_WEB_DEPLOY_SD
esp_err_t init_fs(void)
{
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    gpio_set_pull_mode(15, GPIO_PULLUP_ONLY); // CMD
    gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);  // D0
    gpio_set_pull_mode(4, GPIO_PULLUP_ONLY);  // D1
    gpio_set_pull_mode(12, GPIO_PULLUP_ONLY); // D2
    gpio_set_pull_mode(13, GPIO_PULLUP_ONLY); // D3

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 4,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_card_t *card;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount(CONFIG_RESTFUL_WEB_MOUNT_POINT, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(MODULE_HTTP, "Failed to mount filesystem.");
        } else {
            ESP_LOGE(MODULE_HTTP, "Failed to initialize the card (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }
    /* print card info if mount successfully */
    sdmmc_card_print_info(stdout, card);
    return ESP_OK;
}
#endif

#if CONFIG_RESTFUL_WEB_DEPLOY_SF
esp_err_t init_fs(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = CONFIG_RESTFUL_WEB_MOUNT_POINT,
        .partition_label = "www",
        .max_files = 5,
        .format_if_mount_failed = false,
    };
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(MODULE_HTTP, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(MODULE_HTTP, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(MODULE_HTTP, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(MODULE_HTTP, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(MODULE_HTTP, "Partition size: total: %d, used: %d", total, used);
    }
    return ESP_OK;
}
#endif


static void task_start_restful(void* args)
{
    //ESP_ERROR_CHECK(nvs_flash_init());
    //ESP_ERROR_CHECK(esp_netif_init());
    //ESP_ERROR_CHECK(esp_event_loop_create_default());
    initialise_mdns();
    netbiosns_init();
    netbiosns_set_name(CONFIG_RESTFUL_MDNS_HOST_NAME);

    //ESP_ERROR_CHECK(example_connect());
    ESP_ERROR_CHECK(init_fs());
    //ESP_ERROR_CHECK(start_rest_server(CONFIG_RESTFUL_WEB_MOUNT_POINT));
}

void sanp_sw_rest_init(void)
{
    ESP_ERROR_CHECK(snap_sw_module_start(task_start_restful, false, 0, MODULE_HTTP));
}

