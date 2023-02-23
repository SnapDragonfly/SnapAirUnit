
/// @file msp_protocol.c

/*
 * idf header files
 */
#include "esp_spp_api.h"
#include "esp_system.h"
#include "esp_log.h"
#include "lwip/err.h"

/*
 * basic header files
 */
#include "define.h"
#include "handle.h"

/*
 * module header files
 */
#include "module.h"
#include "mode.h"
#include "hy_protocol.h"

/*
 * service header files
 */
#include "ttl.h"
#include "udp_server.h"
#include "udp_client.h"

#define CHECKXOR(xor, character)   (xor^character)

#define SOF_DIR_FC                         0x68
#define SOF_DIR_PC                         0x58
#define LINK_2                             0xF2
#define LINK_3                             0xF3

#define HY_PORT_INBUF_SIZE                 192


typedef enum {
    HY_IDLE                               = 0,
    HY_HEADER_FUNCTION                    = 1,

    HY_HEADER_LENGTH                      = 2,
    HY_PAYLOAD                            = 3,
    HY_XOR                                = 4,

    HY_COMMAND_RECEIVED                   = 5
} hyState_e;

typedef struct hyPort_s {
    hyState_e c_state;
    uint8_t function;
    uint8_t length;
    uint8_t inBuf[HY_PORT_INBUF_SIZE];
    uint8_t offset;
    uint8_t xor;
} hyPort_t;


static hyPort_t g_esp_hy_port;

uint8_t CheckXOR(uint8_t *buf, uint8_t len, uint8_t xor)
{
    uint8_t i =0;

    if (NULL == buf)
        return 0;

    for(i = 0; i < len; i++)
    {
        xor = CHECKXOR(xor, (*(buf+i)));
    }
    return xor;
}

static bool hy_serial_process_received_data(hyPort_t *hyPort, uint8_t c)
{
    switch (hyPort->c_state) {
        default:
        case HY_IDLE:
            if (c == SOF_DIR_PC) {
                hyPort->c_state = HY_HEADER_FUNCTION;
            }
            else {
                return false;
            }
            break;

        case HY_HEADER_FUNCTION:
            hyPort->function = c;
            hyPort->xor = c;
            hyPort->c_state = HY_HEADER_LENGTH;
            break;

        case HY_HEADER_LENGTH:
            hyPort->length = c;
            hyPort->xor ^= c;
            hyPort->offset = 0;
            hyPort->c_state = HY_PAYLOAD;
            break;

        case HY_PAYLOAD:
            hyPort->inBuf[hyPort->offset++] = c;
            hyPort->xor ^= c;
            //ESP_LOGI(MODULE_HY_PROTO, "offset %d hyPort->length %d", hyPort->offset, hyPort->length);
            if (hyPort->offset == hyPort->length) {
                hyPort->c_state = HY_XOR;
            }
            break;

        case HY_XOR:
            if (hyPort->xor == c) {
                hyPort->c_state = HY_COMMAND_RECEIVED;
            } else {
                hyPort->c_state = HY_IDLE;
                return false;
            }
            break;
    }

#if(DEBUG_HY_PROTO)
    ESP_LOGI(MODULE_HY_PROTO, "ttl hyPort->c_state %d 0x%x", hyPort->c_state, c);
#endif

    return true;
}

esp_err_t ttl_handle_wifi_hy(uint8_t * buf, int len)
{
    if (NULL == buf){
        return ESP_FAIL;
    }

    for(int i = 0; i < len; i++){
        bool hy_ret = hy_serial_process_received_data(&g_esp_hy_port, *(buf +i));
        if(!hy_ret){
            ESP_LOGW(MODULE_HY_PROTO, "state %d, c 0x%x", g_esp_hy_port.c_state, *(buf +i));
        }

        if(HY_COMMAND_RECEIVED == g_esp_hy_port.c_state){
#if(DEBUG_HY_PROTO)
            ESP_LOGI(MODULE_HY_PROTO, "g_esp_hy_port.function 0x%x", g_esp_hy_port.function);
#endif
            switch(g_esp_hy_port.function){
                case LINK_2:
                    (void)udp_control_send(g_esp_hy_port.inBuf, g_esp_hy_port.length);
                    return ESP_OK;

                    break;
                    
                case LINK_3:
                    int n = udp_status_send(g_esp_hy_port.inBuf, g_esp_hy_port.length);
                    if(g_esp_hy_port.length != n){
                        ESP_LOGW(MODULE_HY_PROTO, "udp_status_send failed errno %d", errno);
                    }
                    return ESP_OK;

                    break;

                default:
                    break;
            }
        }
    }

    return ESP_FAIL;
}


esp_err_t udp_handle_hy(uint8_t * buf, int len)
{
    // calc XOR
    uint8_t xor = LINK_2 ^ len;
    xor = CheckXOR(buf, len, xor);

    // Send the frame
    uint8_t character;

    character = SOF_DIR_FC;
    ESP_ERROR_CHECK(ttl_msg_send(&character, 1));
#if(DEBUG_HY_PROTO)
    ESP_LOGI(MODULE_HY_PROTO, "HY SOF:");
    esp_log_buffer_hex(MODULE_HY_PROTO, &character, 1);
#endif

    character = LINK_2;
    ESP_ERROR_CHECK(ttl_msg_send(&character, 1));
#if(DEBUG_HY_PROTO)
    ESP_LOGI(MODULE_HY_PROTO, "HY FUNCTION:");
    esp_log_buffer_hex(MODULE_HY_PROTO, &character, 1);
#endif

    ESP_ERROR_CHECK(ttl_msg_send((uint8_t *)&len, 1));
#if(DEBUG_HY_PROTO)
    ESP_LOGI(MODULE_HY_PROTO, "HY FUNCTION:");
    esp_log_buffer_hex(MODULE_HY_PROTO, &character, 1);
#endif

    ESP_ERROR_CHECK(ttl_msg_send(buf, len));
#if(DEBUG_HY_PROTO)
    ESP_LOGI(MODULE_HY_PROTO, "HY DATA:");
    esp_log_buffer_hex(MODULE_HY_PROTO, buf, len);
#endif

    ESP_ERROR_CHECK(ttl_msg_send(&xor, 1));
#if(DEBUG_HY_PROTO)
    ESP_LOGI(MODULE_HY_PROTO, "HY XOR:");
    esp_log_buffer_hex(MODULE_HY_PROTO, &xor, 1);
#endif

    return ESP_OK;
}



