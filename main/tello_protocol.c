/// @file tello_protocol.c

/*
 * idf header files
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "esp_system.h"
#include "esp_log.h"

/*
 * basic header files
 */
#include "define.h"
#include "handle.h"

/*
 * module header files
 */
#include "module.h"
#include "cmd_udp.h"
#include "tello_protocol.h"

/*
 * service header files
 */
#include "ttl.h"
#include "udp_server.h"

struct udp_command {
    const char command[UDP_COMMAND_LENGTH];
    udp_func   call;
};

struct udp_command g_udp_commands[] = {
    {"bluetooth", udp_bluetooth},
    {"ap",        udp_ap       },
    {"wifi",      udp_wifi     },
    {"sdk?",      udp_sdk      },
    {"arm",       udp_arm      },
    {"emergency", udp_emergency},
    {"reboot",    udp_reboot   },
    {"command",   udp_command  },
    {"exit",      udp_exit     },
    {"identity",  udp_identity },
};

static esp_err_t tello_protocol_parse(uint8_t * buf, int len)
{
    esp_err_t err;
    int argc        = 0;
    char *argv[UDP_PARAMS_COUNT+1]  = { NULL, };

    char *szcmdline = (char *) malloc(len + 1);
    if (szcmdline == NULL) {
        return ESP_ERR_NO_MEM;
    }
    memcpy(szcmdline, buf, len);
    szcmdline[len] = '\0';
    char *offset    = szcmdline;

#if (DEBUG_TELLO_PROTO)
    ESP_LOGI(MODULE_TELLO_PROTO, "tello_protocol_parse %d bytes", len);
    esp_log_buffer_hex(MODULE_TELLO_PROTO, szcmdline, len);
#endif /* DEBUG_TELLO_PROTO */

    if (1 == len && 0 == szcmdline[0]){
        /* 
         * Very rare case, which mightbe comes from  debug tool
         */
        free(szcmdline);
        (void)led_mode_set((struct blink_led *)g_status_handle, LED_STATUS_ERROR);
        return ESP_ERR_NOT_FOUND;
    }

    char seps[]        = " ,\t\n";
    char *token = strtok(offset, seps);
    while (token != NULL) {
        argv[argc++]= token;
        token[-1] = (argc == 1) ? token[-1] : '\0';
        token = strtok(NULL, seps);
    }

    for (int i = 0; i < sizeof(argv)/sizeof(argv[0]); ++i) {
        if (argv[i] == NULL) {
            break;
        }
#if (DEBUG_TELLO_PROTO)
        ESP_LOGI(MODULE_TELLO_PROTO, "%d --> %s", i, argv[i]);
#endif /* DEBUG_TELLO_PROTO */
    }

    int total_commands = sizeof(g_udp_commands)/sizeof(struct udp_command);

#if (DEBUG_TELLO_PROTO)
    ESP_LOGI(MODULE_TELLO_PROTO, "argc(%d) commands(%d)", argc, total_commands);
#endif /* DEBUG_TELLO_PROTO */

    if (argc < 1){
        /* 
         * Very rare case, which mightbe comes from  debug tool
         * I (24611) tel: tello_protocol_parse 3 bytes
         * I (24621) tel: 00 00 00
         * I (24621) tel: argc(0) commands(10)
         */
        free(szcmdline);
        (void)led_mode_set((struct blink_led *)g_status_handle, LED_STATUS_ERROR);
        return ESP_ERR_NOT_FOUND;
    }

    for (int i = 0; i < total_commands; i++){
        if(!strcmp(g_udp_commands[i].command, argv[0])){
            struct udp_data data;

            data.counts = argc - 1;
            for (int j = 1; j < argc; j++){
                data.param[j - 1] = argv[j];
            }

#if (DEBUG_TELLO_PROTO)
            ESP_LOGI(MODULE_TELLO_PROTO, "hit --> %s", argv[0]);
#endif /* DEBUG_TELLO_PROTO */
            err = g_udp_commands[i].call(&data);
            free(szcmdline);
            return err;
        }
#if (DEBUG_TELLO_PROTO)
        else {
            ESP_LOGI(MODULE_TELLO_PROTO, "comparing... %s to %s", argv[0], g_udp_commands[i].command);
        }
#endif /* DEBUG_TELLO_PROTO */
    }

    free(szcmdline);
    return ESP_ERR_NOT_FOUND;
}

esp_err_t nomsp_handle_cli(uint8_t * buf, int len)
{
    if(NULL == buf){
        return ESP_FAIL;
    }

    if (*buf == '$'){
        return ESP_FAIL;
    }

#if (DEBUG_TELLO_PROTO)
    ESP_LOGI(MODULE_TELLO_PROTO, "nomsp_handle_cli %d bytes", len);
    esp_log_buffer_hex(MODULE_TELLO_PROTO, buf, len);
#endif /* DEBUG_TELLO_PROTO */

    ESP_ERROR_CHECK(ttl_msg_send(buf, len));
    return ESP_OK;
}
esp_err_t nomsp_handle_tello(uint8_t * buf, int len)
{
    if(NULL == buf){
        return ESP_FAIL;
    }

    if (*buf == '$'){
        return ESP_FAIL;
    }

    if (*buf == '#'){
#if (DEBUG_TELLO_PROTO)
        ESP_LOGI(MODULE_TELLO_PROTO, "cli char found %d bytes", len);
        esp_log_buffer_hex(MODULE_TELLO_PROTO, buf, len);
#endif /* DEBUG_TELLO_PROTO */
        return ESP_ERR_NOT_SUPPORTED;
    }

    esp_err_t err = tello_protocol_parse(buf, len);
    switch(err){
        case ESP_ERR_NOT_FOUND:
#if (DEBUG_TELLO_PROTO)
            ESP_LOGI(MODULE_TELLO_PROTO, "tello NOT found passthrough %d bytes", len);
            esp_log_buffer_hex(MODULE_TELLO_PROTO, buf, len);
#endif /* DEBUG_TELLO_PROTO */

            ESP_ERROR_CHECK(ttl_msg_send(buf, len));
            break;

        case ESP_ERR_INVALID_RESPONSE:
            /* special feed back response should be handled in command call */
            break;

        case ESP_OK:
            udp_msg_send((uint8_t *)TELLO_RESPONSE_OK, strlen(TELLO_RESPONSE_OK));
            break;

        case ESP_FAIL:
            /* FALL THROUGH */
        case ESP_ERR_NO_MEM:
            /* FALL THROUGH */
        default:
            udp_msg_send((uint8_t *)TELLO_RESPONSE_ERR, strlen(TELLO_RESPONSE_ERR));
            break;
        }

    return err;
}




