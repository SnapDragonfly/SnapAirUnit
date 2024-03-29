/// @file rest_server.c

/*
 * idf header files
 */
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
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "sdmmc_cmd.h"
#if CONFIG_RESTFUL_WEB_DEPLOY_SD
#include "driver/sdmmc_host.h"
#endif
#include "nvs_flash.h"
#include "mdns.h"
#include "lwip/apps/netbiosns.h"

/*
 * basic header files
 */
#include "define.h"
#include "handle.h"

/*
 * module header files
 */
#include "module.h"
#include "cJSON.h"
#include "msp_protocol.h"
#include "mode.h"
#include "factory_setting.h"
#include "process.h"

/*
 * service header files
 */
//TBD


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
static esp_err_t common_get_handler(httpd_req_t *req)
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

#if (DEBUG_HTTP)
    ESP_LOGI(MODULE_HTTP, "File sending complete");
#endif /* DEBUG_HTTP */

    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t wireless_get_handler(httpd_req_t *req)
{
    int mode = wireless_mode_get();

    httpd_resp_set_type(req, "application/json");
    cJSON *root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, "wireless", mode);
    const char *wireless_data_raw = cJSON_Print(root);

#if (DEBUG_HTTP)
    ESP_LOGI(MODULE_HTTP, "wireless data get: %s", wireless_data_raw);
#endif /* DEBUG_HTTP */

    httpd_resp_sendstr(req, wireless_data_raw);
    free((void *)wireless_data_raw);
    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t wireless_post_handler(httpd_req_t *req)
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

    int mode = SW_MODE_NULL;
    cJSON *root = cJSON_Parse(buf);
    struct cJSON *item = cJSON_GetObjectItem(root, "wireless");
    if(NULL != item){
        if(cJSON_Number == item->type){
            mode = item->valueint;
        } else if (cJSON_String == item->type){
            mode = atoi(item->valuestring);
        } else{
            ESP_LOGW(MODULE_HTTP, "Can't be HERE json type 0x%01x", item->type);
        }
    }

    int curr = wireless_mode_get();

#if (DEBUG_HTTP)
    ESP_LOGI(MODULE_HTTP, "wireless: %s, set to %d, curr %d", buf, mode, curr);
#endif /* DEBUG_HTTP */

    if(mode >= SW_MODE_NULL){
        httpd_resp_sendstr(req, RESTFUL_API_RESPONSE_INVALID);
    } else if( curr == mode){
        httpd_resp_sendstr(req, RESTFUL_API_RESPONSE_ERR);
    } else{
        httpd_resp_sendstr(req, RESTFUL_API_RESPONSE_OK);
        ESP_ERROR_CHECK(esp_event_post_to(g_evt_handle, EVT_PROCESS, EVT_KEY_SHORT_PRESSED, &mode, sizeof(mode), portMAX_DELAY));
    }

    cJSON_Delete(root);
    return ESP_OK;
}


static esp_err_t sta_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    cJSON *root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "ssid", get_sta_ssid());
    cJSON_AddStringToObject(root, "pass", get_sta_pass());
    const char *sta_data_raw = cJSON_Print(root);

#if (DEBUG_HTTP)
    ESP_LOGI(MODULE_HTTP, "sta data get: %s", sta_data_raw);
#endif /* DEBUG_HTTP */

    httpd_resp_sendstr(req, sta_data_raw);
    free((void *)sta_data_raw);
    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t sta_post_handler(httpd_req_t *req)
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

    char *ssid = NULL;
    cJSON *root = cJSON_Parse(buf);
    struct cJSON *item_ssid = cJSON_GetObjectItem(root, "ssid");
    if(NULL != item_ssid){
        if (cJSON_String == item_ssid->type){
            ssid = item_ssid->valuestring;
        } else{
            ESP_LOGW(MODULE_HTTP, "Can't be HERE json type 0x%01x", item_ssid->type);
        }
    }

    char *pass = NULL;
    struct cJSON *item_pass = cJSON_GetObjectItem(root, "pass");
    if(NULL != item_pass){
        if (cJSON_String == item_pass->type){
            pass = item_pass->valuestring;
        } else{
            ESP_LOGW(MODULE_HTTP, "Can't be HERE json type 0x%01x", item_pass->type);
        }
    }

    esp_err_t err;
    if(strlen(pass) >= 8){
        err = set_sta_settings(ssid, pass);
        httpd_resp_sendstr(req, RESTFUL_API_RESPONSE_OK);
    } else {
        httpd_resp_sendstr(req, RESTFUL_API_RESPONSE_INVALID);
    }

#if (DEBUG_HTTP)
    ESP_LOGI(MODULE_HTTP, "sta: %s, set to %s %s err %d", buf, ssid, pass, err);
#else
    UNUSED(err);
#endif /* DEBUG_HTTP */

    cJSON_Delete(root);
    
    return ESP_OK;
}


static esp_err_t ap_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    cJSON *root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "ssid", get_ap_ssid());
    cJSON_AddStringToObject(root, "pass", get_ap_pass());
    const char *ap_data_raw = cJSON_Print(root);

#if (DEBUG_HTTP)
    ESP_LOGI(MODULE_HTTP, "ap data get: %s", ap_data_raw);
#endif /* DEBUG_HTTP */

    httpd_resp_sendstr(req, ap_data_raw);
    free((void *)ap_data_raw);
    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t ap_post_handler(httpd_req_t *req)
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

    char *ssid = NULL;
    cJSON *root = cJSON_Parse(buf);
    struct cJSON *item_ssid = cJSON_GetObjectItem(root, "ssid");
    if(NULL != item_ssid){
        if (cJSON_String == item_ssid->type){
            ssid = item_ssid->valuestring;
        } else{
            ESP_LOGW(MODULE_HTTP, "Can't be HERE json type 0x%01x", item_ssid->type);
        }
    }

    char *pass = NULL;
    struct cJSON *item_pass = cJSON_GetObjectItem(root, "pass");
    if(NULL != item_pass){
        if (cJSON_String == item_pass->type){
            pass = item_pass->valuestring;
        } else{
            ESP_LOGW(MODULE_HTTP, "Can't be HERE json type 0x%01x", item_pass->type);
        }
    }

    esp_err_t err;
    if(strlen(pass) >= 8){
        err = set_ap_settings(ssid, pass);
        httpd_resp_sendstr(req, RESTFUL_API_RESPONSE_OK);
    } else {
        httpd_resp_sendstr(req, RESTFUL_API_RESPONSE_INVALID);
    }

#if (DEBUG_HTTP)
    ESP_LOGI(MODULE_HTTP, "ap: %s, set to %s %s err %d", buf, ssid, pass, err);
#else
    UNUSED(err);
#endif /* DEBUG_HTTP */

    cJSON_Delete(root);
    httpd_resp_sendstr(req, RESTFUL_API_RESPONSE_OK);
    return ESP_OK;
}


static esp_err_t command_get_handler(httpd_req_t *req)
{
    int mode = snap_sw_command_get();

    httpd_resp_set_type(req, "application/json");
    cJSON *root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, "mode", mode);
    const char *command_data_raw = cJSON_Print(root);

#if (DEBUG_HTTP)
    ESP_LOGI(MODULE_HTTP, "command data get: %s", command_data_raw);
#endif /* DEBUG_HTTP */

    httpd_resp_sendstr(req, command_data_raw);
    free((void *)command_data_raw);
    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t command_post_handler(httpd_req_t *req)
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

    int mode = 0;
    cJSON *root = cJSON_Parse(buf);
    struct cJSON *item = cJSON_GetObjectItem(root, "mode");
    if(NULL != item){
        if(cJSON_Number == item->type){
            mode = item->valueint;
        } else if (cJSON_String == item->type){
            mode = atoi(item->valuestring);
        } else{
            ESP_LOGW(MODULE_HTTP, "Can't be HERE json type 0x%01x", item->type);
        }
    }

    snap_sw_command_set(mode);

#if (DEBUG_HTTP)
    ESP_LOGI(MODULE_HTTP, "command: %s, set to %d", buf, mode);
#endif /* DEBUG_HTTP */

    cJSON_Delete(root);
    httpd_resp_sendstr(req, RESTFUL_API_RESPONSE_OK);
    return ESP_OK;
}


extern uint16_t g_esp_rc_channel[];

static esp_err_t rc_data_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    cJSON *root = cJSON_CreateObject();

    for(int i = 0; i < MAX_SUPPORTED_RC_CHANNEL_COUNT; i++){
    
        char nth_rc_channel[CHANNEL_TAG_LENGTH];
        snprintf(nth_rc_channel, CHANNEL_TAG_LENGTH, "ch_%d", i);
        nth_rc_channel[CHANNEL_TAG_LENGTH-1] = 0;
        
        cJSON_AddNumberToObject(root, nth_rc_channel, g_esp_rc_channel[i]);
#if (DEBUG_HTTP)
        ESP_LOGI(MODULE_HTTP, "RC get channel %d value %d", i, g_esp_rc_channel[i]);
#endif /* DEBUG_HTTP */
    }
    const char *rc_data_raw = cJSON_Print(root);

#if (DEBUG_HTTP)
    ESP_LOGI(MODULE_HTTP, "RC data get: %s", rc_data_raw);
#endif /* DEBUG_HTTP */

    httpd_resp_sendstr(req, rc_data_raw);
    free((void *)rc_data_raw);
    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t rc_data_post_handler(httpd_req_t *req)
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

#if (DEBUG_HTTP)
    ESP_LOGI(MODULE_HTTP, "RC data post: %s", buf);
#endif /* DEBUG_HTTP */

    cJSON *root = cJSON_Parse(buf);
    for(int i = 0; i < MAX_SUPPORTED_RC_CHANNEL_COUNT; i++){
        char nth_rc_channel[CHANNEL_TAG_LENGTH];
        snprintf(nth_rc_channel, CHANNEL_TAG_LENGTH, "ch_%d", i);
        nth_rc_channel[CHANNEL_TAG_LENGTH-1] = 0;

        struct cJSON *item = cJSON_GetObjectItem(root, nth_rc_channel);
        if(NULL == item){
            continue;
        }

        if(cJSON_Number == item->type){
            g_esp_rc_channel[i] = item->valueint;
        } else if (cJSON_String == item->type){
            g_esp_rc_channel[i] = atoi(item->valuestring);
        } else{
            ESP_LOGW(MODULE_HTTP, "Can't be HERE json type 0x%01x", item->type);
        }
#if (DEBUG_HTTP)
        ESP_LOGI(MODULE_HTTP, "RC post channel %d value %d", i, g_esp_rc_channel[i]);
#endif /* DEBUG_HTTP */
    }

    cJSON_Delete(root);
    httpd_resp_sendstr(req, RESTFUL_API_RESPONSE_OK);
    return ESP_OK;
}


/* Simple handler for getting system handler */
static esp_err_t system_info_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    cJSON *root = cJSON_CreateObject();
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    cJSON_AddStringToObject(root, "sdk_version", get_idf_versions());
    cJSON_AddStringToObject(root, "app_version", get_app_versions());
    cJSON_AddNumberToObject(root, "model", chip_info.model);
    cJSON_AddNumberToObject(root, "cores", chip_info.cores);
    cJSON_AddNumberToObject(root, "revision", chip_info.revision);
    cJSON_AddNumberToObject(root, "simplified", WIRELESS_SIMPLIFIED);
    const char *sys_info = cJSON_Print(root);
    httpd_resp_sendstr(req, sys_info);
    free((void *)sys_info);
    cJSON_Delete(root);
    return ESP_OK;
}


static httpd_handle_t g_rest_server = NULL;

rest_server_context_t g_rest_context;


/* URI handler for getting web server files */
httpd_uri_t common_get_uri = {
	.uri = "/*",
	.method = HTTP_GET,
	.handler = common_get_handler,
	.user_ctx = &g_rest_context
};

/* URI handler for posting command */
httpd_uri_t wireless_post_uri = {
	.uri = "/api/v1/wireless/post",
	.method = HTTP_POST,
	.handler = wireless_post_handler,
	.user_ctx = &g_rest_context
};

/* URI handler for fetching command */
httpd_uri_t wireless_get_uri = {
	.uri = "/api/v1/wireless/raw",
	.method = HTTP_GET,
	.handler = wireless_get_handler,
	.user_ctx = &g_rest_context
};


/* URI handler for posting  command */
httpd_uri_t sta_post_uri = {
	.uri = "/api/v1/sta/post",
	.method = HTTP_POST,
	.handler = sta_post_handler,
	.user_ctx = &g_rest_context
};

/* URI handler for fetching command */
httpd_uri_t sta_get_uri = {
	.uri = "/api/v1/sta/raw",
	.method = HTTP_GET,
	.handler = sta_get_handler,
	.user_ctx = &g_rest_context
};


/* URI handler for posting  command */
httpd_uri_t ap_post_uri = {
	.uri = "/api/v1/ap/post",
	.method = HTTP_POST,
	.handler = ap_post_handler,
	.user_ctx = &g_rest_context
};

/* URI handler for fetching command */
httpd_uri_t ap_get_uri = {
	.uri = "/api/v1/ap/raw",
	.method = HTTP_GET,
	.handler = ap_get_handler,
	.user_ctx = &g_rest_context
};

/* URI handler for posting command */
httpd_uri_t command_post_uri = {
	.uri = "/api/v1/command/post",
	.method = HTTP_POST,
	.handler = command_post_handler,
	.user_ctx = &g_rest_context
};

/* URI handler for fetching command */
httpd_uri_t command_get_uri = {
	.uri = "/api/v1/command/raw",
	.method = HTTP_GET,
	.handler = command_get_handler,
	.user_ctx = &g_rest_context
};

/* URI handler for posting rc data */
httpd_uri_t rc_data_post_uri = {
	.uri = "/api/v1/rc/post",
	.method = HTTP_POST,
	.handler = rc_data_post_handler,
	.user_ctx = &g_rest_context
};

/* URI handler for fetching rc data */
httpd_uri_t rc_data_get_uri = {
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

esp_err_t rest_srv_start(const char *base_path)
{
    REST_CHECK(base_path, "wrong base path", err);
    //REST_CHECK(rest_context, "No memory for rest context", err);
    strlcpy(g_rest_context.base_path, base_path, sizeof(g_rest_context.base_path));

    httpd_config_t config    = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers  = RESTFUL_API_MAX_URI_HANDLERS;
    config.uri_match_fn      = httpd_uri_match_wildcard;

    ESP_LOGI(MODULE_HTTP, "Starting HTTP Server");

    REST_CHECK(httpd_start(&g_rest_server, &config) == ESP_OK, "Start server failed", err);

    httpd_register_uri_handler(g_rest_server, &system_info_get_uri);

    httpd_register_uri_handler(g_rest_server, &rc_data_get_uri);

    httpd_register_uri_handler(g_rest_server, &rc_data_post_uri);

    httpd_register_uri_handler(g_rest_server, &command_get_uri);

    httpd_register_uri_handler(g_rest_server, &command_post_uri);

    httpd_register_uri_handler(g_rest_server, &ap_get_uri);

    httpd_register_uri_handler(g_rest_server, &ap_post_uri);

    httpd_register_uri_handler(g_rest_server, &sta_get_uri);

    httpd_register_uri_handler(g_rest_server, &sta_post_uri);

    httpd_register_uri_handler(g_rest_server, &wireless_get_uri);

    httpd_register_uri_handler(g_rest_server, &wireless_post_uri);

    httpd_register_uri_handler(g_rest_server, &common_get_uri);

    return ESP_OK;
err:
    return ESP_FAIL;
}

esp_err_t rest_srv_stop(void)
{
    ESP_LOGI(MODULE_HTTP, "Stoping HTTP Server");

    httpd_unregister_uri_handler(g_rest_server, system_info_get_uri.uri, system_info_get_uri.method);

    httpd_unregister_uri_handler(g_rest_server, rc_data_get_uri.uri, rc_data_get_uri.method);

    httpd_unregister_uri_handler(g_rest_server, rc_data_post_uri.uri, rc_data_post_uri.method);

    httpd_unregister_uri_handler(g_rest_server, command_get_uri.uri, command_get_uri.method);

    httpd_unregister_uri_handler(g_rest_server, command_post_uri.uri, command_post_uri.method);

    httpd_unregister_uri_handler(g_rest_server, ap_get_uri.uri, ap_get_uri.method);

    httpd_unregister_uri_handler(g_rest_server, ap_post_uri.uri, ap_post_uri.method);

    httpd_unregister_uri_handler(g_rest_server, sta_get_uri.uri, sta_get_uri.method);

    httpd_unregister_uri_handler(g_rest_server, sta_post_uri.uri, sta_post_uri.method);

    httpd_unregister_uri_handler(g_rest_server, wireless_get_uri.uri, wireless_get_uri.method);

    httpd_unregister_uri_handler(g_rest_server, wireless_post_uri.uri, wireless_post_uri.method);

    httpd_unregister_uri_handler(g_rest_server, common_get_uri.uri, common_get_uri.method);

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

void rest_srv_init(void* args)
{
    initialise_mdns();
    netbiosns_init();
    netbiosns_set_name(CONFIG_RESTFUL_MDNS_HOST_NAME);

    ESP_ERROR_CHECK(init_fs());
}


