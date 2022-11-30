
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "esp_system.h"
#include "esp_log.h"

#include "module.h"
#include "define.h"
#include "ttl.h"
#include "udp_server.h"
#include "cmd_udp.h"
#include "tello_protocol.h"

struct udp_command {
    const char command[UDP_COMMAND_LENGTH];
    udp_func   call;
};

struct udp_command g_udp_commands[] = {
    {"bluetooth", udp_bluetooth},
    {"ap",        udp_ap       },
    {"wifi",      udp_wifi     },
};

static esp_err_t tello_protocol_parse(uint8_t * buf, int len)
{
    int argc        = 0;
    char *argv[UDP_PARAMS_COUNT+1]  = { NULL, };

    char *szcmdline = (char *) malloc(len + 1);
    if (szcmdline == NULL) {
        return ESP_FAIL;
    }
    memcpy(szcmdline, buf, len);
    szcmdline[len] = '\0';
    char *offset    = szcmdline;

#if (DEBUG_TELLO_PROTO)
    ESP_LOGI(MODULE_TELLO_PROTO, "tello_protocol_parse %d bytes", len);
    esp_log_buffer_hex(MODULE_TELLO_PROTO, szcmdline, len);
#endif /* DEBUG_TELLO_PROTO */

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
    for (int i = 0; i < total_commands; i++){
#if (DEBUG_TELLO_PROTO)
        ESP_LOGI(MODULE_TELLO_PROTO, "comparing... %s to %s", argv[0], g_udp_commands[i].command);
        esp_log_buffer_hex(MODULE_TELLO_PROTO, argv[0], strlen(argv[0]));
#endif /* DEBUG_TELLO_PROTO */
        if(!strcmp(g_udp_commands[i].command, argv[0])){
            struct udp_data data;
            
            data.counts = argc - 1;
            for (int j = 1; j < argc; j++){
                data.param[j - 1] = argv[j];
            }
            
            return g_udp_commands[i].call(&data);
        }
    }

    free(szcmdline);
    return ESP_FAIL;
}

esp_err_t udp_handle_tello_protocol(uint8_t * buf, int len)
{
    if(NULL == buf){
        return ESP_FAIL;
    }

    if (*buf == '$'){
        return ESP_FAIL;
    }

    esp_err_t err = tello_protocol_parse(buf, len);
    if(ESP_OK != err){
#if (DEBUG_TELLO_PROTO)
        ESP_LOGI(MODULE_TELLO_PROTO, "udp_handle %d bytes", len);
        esp_log_buffer_hex(MODULE_TELLO_PROTO, buf, len);
        //ESP_LOGI(MODULE_TELLO_PROTO, "%s", buf);
#endif /* DEBUG_TELLO_PROTO */

        ESP_ERROR_CHECK(ttl_send(buf, len));
    }
    return err;
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


