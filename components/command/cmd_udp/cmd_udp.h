
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define UDP_COMMAND_LENGTH     16
#define UDP_PARAMS_COUNT       8

struct udp_data {
    const char * param[UDP_PARAMS_COUNT];
    uint8_t      counts;
};

typedef esp_err_t (*udp_func)(struct udp_data *); 

esp_err_t udp_bluetooth(struct udp_data * data);
esp_err_t udp_ap(struct udp_data * data);
esp_err_t udp_wifi(struct udp_data * data);
esp_err_t udp_sdk(struct udp_data * data);
esp_err_t udp_arm(struct udp_data * data);
esp_err_t udp_emergency(struct udp_data * data);
esp_err_t udp_command(struct udp_data * data);
esp_err_t udp_reboot(struct udp_data * data);
esp_err_t udp_exit(struct udp_data * data);
esp_err_t udp_identity(struct udp_data * data);
esp_err_t udp_loopback(struct udp_data * data);


#ifdef __cplusplus
}
#endif
