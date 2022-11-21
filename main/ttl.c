
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "esp_log.h"
#include "esp_spp_api.h"
#include "esp_bt_device.h"
#include "esp_bt_defs.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"

#include "module.h"
#include "ttl.h"

#define TTL_UART_TXD        (CONFIG_MSP_TTL_TXD)
#define TTL_UART_RXD        (CONFIG_MSP_TTL_RXD)
#define TTL_BUF_SIZE        (1024)

QueueHandle_t ttl_uart_queue = NULL;

extern bool bt_is_connected;
extern uint32_t esp_ssp_handle;

static void ttl_uart_task(void *pvParameters)
{
    uart_event_t event;

    for (;;) {
        //Waiting for UART event.
        if (xQueueReceive(ttl_uart_queue, (void * )&event, (TickType_t)portMAX_DELAY)) {
            switch (event.type) {
            //Event of UART receving data
            case UART_DATA:
                if ((event.size)&&(bt_is_connected)) {
                    uint8_t * temp = NULL;

                    temp = (uint8_t *)malloc(sizeof(uint8_t)*event.size);
                    if(temp == NULL){
                        ESP_LOGE(MODULE_UART, "%s malloc.1 failed\n", __func__);
                        break;
                    }
                    memset(temp,0x0,event.size);
                    uart_read_bytes(TTL_UART_NUM, temp, event.size, portMAX_DELAY);

                    ESP_LOGI(MODULE_UART, "ttl read %d Bytes\n", event.size);
                    if (event.size < 128) {
                        esp_log_buffer_hex(MODULE_UART, temp, event.size);
                    }

                    // To do send to BT SPP
                    if (esp_ssp_handle){
                        esp_spp_write(esp_ssp_handle, event.size, temp);
                    }
                    //esp_ble_gatts_send_indicate(spp_gatts_if, spp_conn_id, spp_handle_table[SPP_IDX_SPP_DATA_NTY_VAL],event.size, temp, false);
                    free(temp);
                }
                break;
            default:
                break;
            }
        }
    }
    vTaskDelete(NULL);
}

void ttl_uart_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_RTS,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_DEFAULT,
    };

    //Install UART driver, and get the queue.
    uart_driver_install(TTL_UART_NUM, TTL_BUF_SIZE * 2, 8192, 10, &ttl_uart_queue, 0);
    //Set UART parameters
    uart_param_config(TTL_UART_NUM, &uart_config);
    //Set UART pins
    uart_set_pin(TTL_UART_NUM, TTL_UART_TXD, TTL_UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    xTaskCreate(ttl_uart_task, MODULE_UART, 2048, (void*)TTL_UART_NUM, 8, NULL);
}


