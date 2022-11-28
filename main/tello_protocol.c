
#include "esp_system.h"
#include "esp_log.h"

#include "module.h"
#include "define.h"
#include "ttl.h"
#include "udp_server.h"
#include "tello_protocol.h"

esp_err_t udp_handle_tello_protocol(uint8_t * buf, int len)
{
    if(NULL == buf){
        return ESP_FAIL;
    }

    if (*buf == '$'){
        return ESP_FAIL;
    }

#if 1

#if (DEBUG_TELLO_PROTO)
    ESP_LOGI(MODULE_TELLO_PROTO, "udp_handle %d bytes", len);
    esp_log_buffer_hex(MODULE_TELLO_PROTO, buf, len);
    //ESP_LOGI(MODULE_TELLO_PROTO, "%s", buf);
#endif /* DEBUG_TELLO_PROTO */

    ESP_ERROR_CHECK(ttl_send(buf, len));
    return ESP_OK;

#else
    ESP_LOGW(MODULE_TELLO_PROTO, "Tello protocol need implementation!!!");
    return ESP_FAIL;
#endif
}

esp_err_t ttl_handle_tello_protocol(uint8_t * buf, int len)
{
    if(NULL == buf){
        return ESP_FAIL;
    }

#if (DEBUG_TELLO_PROTO)
    ESP_LOGI(MODULE_TELLO_PROTO, "ttl_handle %d bytes", len);
    esp_log_buffer_hex(MODULE_TELLO_PROTO, buf, len);
    //ESP_LOGI(MODULE_TELLO_PROTO, "%s", buf);
#endif /* DEBUG_TELLO_PROTO */

    udp_send_msg(buf, len);
    return ESP_OK;
}


