/// @file udp_server.c

/*
 * idf header files
 */
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

/*
 * basic header files
 */
#include "config.h"
#include "define.h"

/*
 * module header files
 */
#include "msp_protocol.h"
#include "tello_protocol.h"
#include "module.h"
#include "mode.h"

/*
 * service header files
 */
#include "ttl.h"

static int                   g_server_sock;
static struct sockaddr_storage g_source_addr;
char                          g_addr_str[STR_IP_LEN];

esp_err_t udp_msg_send(uint8_t * buf, int len)
{
#if (DEBUG_UDP_SRV)
    ESP_LOGI(MODULE_UDP_SRV, "sending %d bytes", len);
    esp_log_buffer_hex(MODULE_UDP_SRV, buf, len);
#endif /* DEBUG_UDP_SRV */

    int err = sendto(g_server_sock, buf, len, 0, (struct sockaddr *)&g_source_addr, sizeof(g_source_addr));
    if (err < 0) {
        protocol_state_degrade(SW_STATE_HALF_DUPLEX);
        ESP_LOGE(MODULE_UDP_SRV, "Error occurred during sending: errno %d", errno);
    }

    return err;
}

static void udp_srv_task(void *pvParameters)
{
    char rx_buffer[STR_BUFFER_LEN];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;

    memset(g_addr_str, 0, STR_IP_LEN);
    while (1) {

        if(SW_STATE_INVALID == protocol_state_get() || SW_MODE_BT_SPP == wireless_mode_get()){
#if (DEBUG_UDP_SRV)
            ESP_LOGI(MODULE_UDP_SRV, "invalid switching time");
#endif /* DEBUG_UDP_SRV */
            vTaskDelay(TIME_ONE_SECOND_IN_MS / portTICK_PERIOD_MS);
            continue;
        }

        if (addr_family == AF_INET) {
            struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
            dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
            dest_addr_ip4->sin_family = AF_INET;
            dest_addr_ip4->sin_port = htons(CONTROL_PORT);
            ip_protocol = IPPROTO_IP;
        } else if (addr_family == AF_INET6) {
            bzero(&dest_addr.sin6_addr.un, sizeof(dest_addr.sin6_addr.un));
            dest_addr.sin6_family = AF_INET6;
            dest_addr.sin6_port = htons(CONTROL_PORT);
            ip_protocol = IPPROTO_IPV6;
        }

        g_server_sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (g_server_sock < 0) {
            ESP_LOGE(MODULE_UDP_SRV, "Unable to create socket: errno %d", errno);
            break;
        }
#if (DEBUG_UDP_SRV)
        ESP_LOGI(MODULE_UDP_SRV, "Socket created");
#endif /* DEBUG_UDP_SRV */

#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
        int enable = 1;
        lwip_setsockopt(g_server_sock, IPPROTO_IP, IP_PKTINFO, &enable, sizeof(enable));
#endif

#if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
        if (addr_family == AF_INET6) {
            // Note that by default IPV6 binds to both protocols, it is must be disabled
            // if both protocols used at the same time (used in CI)
            int opt = 1;
            setsockopt(g_server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            setsockopt(g_server_sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
        }
#endif
        // Set timeout
        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        setsockopt (g_server_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

        int err = bind(g_server_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0) {
            ESP_LOGE(MODULE_UDP_SRV, "Socket unable to bind: errno %d", errno);
        }
#if (DEBUG_UDP_SRV)
        ESP_LOGI(MODULE_UDP_SRV, "Socket bound, port %d", CONTROL_PORT);
#endif /* DEBUG_UDP_SRV */
        socklen_t socklen = sizeof(g_source_addr);

#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
        struct iovec iov;
        struct msghdr msg;
        struct cmsghdr *cmsgtmp;
        u8_t cmsg_buf[CMSG_SPACE(sizeof(struct in_pktinfo))];

        iov.iov_base = rx_buffer;
        iov.iov_len = sizeof(rx_buffer);
        msg.msg_control = cmsg_buf;
        msg.msg_controllen = sizeof(cmsg_buf);
        msg.msg_flags = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_name = (struct sockaddr *)&g_source_addr;
        msg.msg_namelen = socklen;
#endif

        while (1) {
#if (DEBUG_UDP_SRV)
            ESP_LOGI(MODULE_UDP_SRV, "Waiting for data");
#endif /* DEBUG_UDP_SRV */
            memset(rx_buffer, 0, STR_BUFFER_LEN);

#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
            int len = recvmsg(g_server_sock, &msg, 0);
#else
            int len = recvfrom(g_server_sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&g_source_addr, &socklen);
#endif
            // Error occurred during receiving
            if (len < 0) {
                //protocol_state_set(SW_STATE_IDLE);
#if (DEBUG_UDP_SRV)
                ESP_LOGE(MODULE_UDP_SRV, "recvfrom failed: errno %d", errno);
#endif /* DEBUG_UDP_SRV */
                break;
            }
            // Data received
            else {
                protocol_state_upgrade(SW_STATE_FULL_DUPLEX);
                // Get the sender's ip address as string
                if (g_source_addr.ss_family == PF_INET) {
                    inet_ntoa_r(((struct sockaddr_in *)&g_source_addr)->sin_addr, g_addr_str, sizeof(g_addr_str) - 1);
#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
                    for ( cmsgtmp = CMSG_FIRSTHDR(&msg); cmsgtmp != NULL; cmsgtmp = CMSG_NXTHDR(&msg, cmsgtmp) ) {
                        if ( cmsgtmp->cmsg_level == IPPROTO_IP && cmsgtmp->cmsg_type == IP_PKTINFO ) {
                            struct in_pktinfo *pktinfo;
                            pktinfo = (struct in_pktinfo*)CMSG_DATA(cmsgtmp);
#if (DEBUG_UDP_SRV)
                            ESP_LOGI(MODULE_UDP_SRV, "dest ip: %s\n", inet_ntoa(pktinfo->ipi_addr));
#endif /* DEBUG_UDP_SRV */
                        }
                    }
#endif
                } else if (g_source_addr.ss_family == PF_INET6) {
                    inet6_ntoa_r(((struct sockaddr_in6 *)&g_source_addr)->sin6_addr, g_addr_str, sizeof(g_addr_str) - 1);
                }

                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...
                esp_err_t ret;
                switch(protocol_state_get()){
                    case SW_STATE_FULL_DUPLEX:
#if (DEBUG_UDP_SRV)
                        ESP_LOGI(MODULE_UDP_SRV, "msp received %d bytes from %s:", len, g_addr_str);
                        esp_log_buffer_hex(MODULE_UDP_SRV, rx_buffer, len);
#endif /* DEBUG_UDP_SRV */

                        ret = wireless_handle_msp((uint8_t *)rx_buffer, len);
                        if(ESP_OK == ret){
                            break;
                        }

                        /* FALL THROUGH */

                    case SW_STATE_TELLO:
                        protocol_state_upgrade(SW_STATE_TELLO);
#if (DEBUG_UDP_SRV)
                        ESP_LOGI(MODULE_UDP_SRV, "tello received %d bytes from %s:", len, g_addr_str);
                        ESP_LOGI(MODULE_UDP_SRV, "%s", rx_buffer);
#endif /* DEBUG_UDP_SRV */

                        ret = nomsp_handle_tello((uint8_t *)rx_buffer, len);
                        if(ESP_ERR_NOT_SUPPORTED == ret){
                            protocol_state_upgrade(SW_STATE_CLI);
                            //vTaskDelay(TIME_50_MS / portTICK_PERIOD_MS);
                            ESP_ERROR_CHECK(ttl_msg_send((uint8_t *)rx_buffer, len));
                        } else if(ESP_OK != ret){
                            protocol_state_degrade(SW_STATE_FULL_DUPLEX);
                        }

                        break;

                    case SW_STATE_CLI:
                        ret = nomsp_handle_cli((uint8_t *)rx_buffer, len);
                        if(ESP_OK != ret){
                            protocol_state_set(SW_STATE_FULL_DUPLEX);
                        }

                        break;

                    default:
                        ESP_LOGW(MODULE_UDP_SRV, "Can't be HERE!!! Received %d bytes from %s:", len, g_addr_str);
                        ESP_LOGW(MODULE_UDP_SRV, "%s", rx_buffer);

                        break;
                    }
            }
        }

        if (g_server_sock != -1) {
#if (DEBUG_UDP_SRV)
            ESP_LOGE(MODULE_UDP_SRV, "Shutting down socket and restarting...");
#endif /* DEBUG_UDP_SRV */
            shutdown(g_server_sock, 0);
            close(g_server_sock);
            //protocol_state_set(SW_STATE_IDLE);
        }
    }
    vTaskDelete(NULL);
}

esp_err_t udp_srv_start(void)
{
#ifdef CONFIG_EXAMPLE_IPV4
    xTaskCreate(udp_srv_task, MODULE_UDP_SRV, TASK_BUFFER_4K0, (void*)AF_INET, 5, NULL);
#endif
#ifdef CONFIG_EXAMPLE_IPV6
    xTaskCreate(udp_srv_task, MODULE_UDP_SRV, TASK_BUFFER_4K0, (void*)AF_INET6, 5, NULL);
#endif

    return ESP_OK;
}

