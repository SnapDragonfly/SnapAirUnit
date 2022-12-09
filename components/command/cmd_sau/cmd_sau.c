

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_err.h"
#include "cmd_sau.h"
#include "cmd_udp.h"

#include "msp_protocol.h"
#include "module.h"
#include "mode.h"
#include "define.h"

static struct {
    struct arg_str *mode;
    struct arg_end *end;
} mode_args;

static struct {
    struct arg_end *end;
} status_args;

static struct {
    struct arg_end *end;
} bluetooth_args;

static struct {
    struct arg_str *ssid;
    struct arg_str *pass;
    struct arg_end *end;
} wifi_ap_args;

static struct {
    struct arg_str *ssid;
    struct arg_str *pass;
    struct arg_end *end;
} wifi_sta_args;

static struct {
    struct arg_end *end;
} sdk_args;

static struct {
    struct arg_end *end;
} emergency_args;

static struct {
    struct arg_str *channel;
    struct arg_str *value;
    struct arg_end *end;
} channel_args;


static int sau_switch(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &mode_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, mode_args.end, argv[0]);
        return 1;
    }

    enum_mode_t mode = (enum_mode_t)strtol(mode_args.mode->sval[0], NULL, 0);

    esp_err_t err = snap_sw_mode_switch(mode);

    if (err != ESP_OK) {
        ESP_LOGE(MODULE_CMD_SAU, "%s", esp_err_to_name(err));
        return 1;
    }

    return 0;
}

static int sau_status(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &status_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, status_args.end, argv[0]);
        return 1;
    }

    enum_state_t state = snap_sw_state_get();
    enum_mode_t mode   = snap_sw_mode_get();

    ESP_LOGI(MODULE_CMD_SAU, "Application mode(%d) state(%d)", mode, state);
    ESP_LOGI(MODULE_CMD_SAU, "Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

    return 0;
}

static int sau_bluetooth(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &bluetooth_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, bluetooth_args.end, argv[0]);
        return 1;
    }

    esp_err_t err = udp_bluetooth(NULL);
    if (err != ESP_OK) {
        ESP_LOGE(MODULE_CMD_SAU, "%s", esp_err_to_name(err));
        return 1;
    }

    return 0;
}

static int sau_ap(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &wifi_ap_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, wifi_ap_args.end, argv[0]);
        return 1;
    }

    const char *ssid = wifi_ap_args.ssid->sval[0];
    const char *pass = wifi_ap_args.pass->sval[0];

    struct udp_data data;
    data.param[0] = ssid;
    data.param[1] = pass;
    data.counts   = 2;

    esp_err_t err = udp_ap((void *)&data);
    if (err != ESP_OK) {
        ESP_LOGE(MODULE_CMD_SAU, "%s", esp_err_to_name(err));
        return 1;
    }

    return 0;
}

static int sau_wifi(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &wifi_sta_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, wifi_sta_args.end, argv[0]);
        return 1;
    }

    const char *ssid = wifi_sta_args.ssid->sval[0];
    const char *pass = wifi_sta_args.pass->sval[0];

    struct udp_data data;
    data.param[0] = ssid;
    data.param[1] = pass;
    data.counts   = 2;

    esp_err_t err = udp_wifi((void *)&data);
    if (err != ESP_OK) {
        ESP_LOGE(MODULE_CMD_SAU, "%s", esp_err_to_name(err));
        return 1;
    }

    return 0;
}

static int sau_sdk(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &sdk_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, sdk_args.end, argv[0]);
        return 1;
    }

    ESP_LOGI(MODULE_CMD_SAU, "Version: SDK(%s) APP(%s)", IDF_VER, APP_VERSION);
    return 0;
}

static int sau_emergency(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &emergency_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, emergency_args.end, argv[0]);
        return 1;
    }

    mspSetChannel(4, 1200);
    mspUpdateChannels();

    ESP_LOGI(MODULE_CMD_SAU, "emergency");
    return 0;
}

static int sau_channel(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &channel_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, status_args.end, argv[0]);
        return 1;
    }

    uint8_t channel = strtol(channel_args.channel->sval[0], NULL, 0);
    uint16_t value  = strtol(channel_args.value->sval[0], NULL, 0);

    if(channel >= MAX_SUPPORTED_RC_CHANNEL_COUNT &&
       value < 900 &&
       value > 2100){
       return 1;
    }

    mspSetChannel(channel, value);
    mspUpdateChannels();

    ESP_LOGI(MODULE_CMD_SAU, "channel %d %d", channel, value);
    return 0;
}


void register_sau(void)
{
    mode_args.mode         = arg_str1(NULL, NULL, "<mode>", "application mode to be set");
    mode_args.end          = arg_end(2);

    status_args.end        = arg_end(2);

    bluetooth_args.end     = arg_end(2);

    wifi_ap_args.ssid      = arg_str1(NULL, NULL, "<ssid>", "wifi ssid");
    wifi_ap_args.pass      = arg_str1(NULL, NULL, "<pass>", "wifi password");
    wifi_ap_args.end       = arg_end(2);

    wifi_sta_args.ssid     = arg_str1(NULL, NULL, "<ssid>", "wifi ssid");
    wifi_sta_args.pass     = arg_str1(NULL, NULL, "<pass>", "wifi password");
    wifi_sta_args.end      = arg_end(2);

    sdk_args.end           = arg_end(2);

    emergency_args.end     = arg_end(2);

    channel_args.channel   = arg_str1(NULL, NULL, "<channel>", "rc channel");
    channel_args.value     = arg_str1(NULL, NULL, "<value>", "channel value");
    channel_args.end       = arg_end(2);

    const esp_console_cmd_t switch_cmd = {
        .command = "switch",
        .help = "Switch <mode>, mode:ap(0)/sta(1)/bt(2).\n"
        "Examples:\n"
        " switch 0 \n"
        " switch 1 \n"
        " switch 2 \n",
        .hint = NULL,
        .func = &sau_switch,
        .argtable = &mode_args
    };

    const esp_console_cmd_t status_cmd = {
        .command = "status",
        .help = "Get status of the application.\n"
        "Examples:\n"
        " status \n",
        .hint = NULL,
        .func = &sau_status,
        .argtable = &status_args
    };

    const esp_console_cmd_t bluetooth_cmd = {
        .command = "bluetooth",
        .help = "Switch to BT SPP Uart.\n"
        "Examples:\n"
        " bluetooth \n",
        .hint = NULL,
        .func = &sau_bluetooth,
        .argtable = &bluetooth_args
    };

    const esp_console_cmd_t ap_cmd = {
        .command = "ap",
        .help = "Switch to WiFi Station.\n"
        "Examples:\n"
        " ap ssid pass \n",
        .hint = NULL,
        .func = &sau_ap,
        .argtable = &wifi_ap_args
    };

    const esp_console_cmd_t wifi_cmd = {
        .command = "wifi",
        .help = "wifi <ssid> <pass>, Switch to WiFi AP.\n"
        "Examples:\n"
        " wifi SnapAirUnit 12345678 \n",
        .hint = NULL,
        .func = &sau_wifi,
        .argtable = &wifi_sta_args
    };

    const esp_console_cmd_t sdk_cmd = {
        .command = "sdk?",
        .help = "Get application version.\n"
        "Examples:\n"
        " sdk? \n",
        .hint = NULL,
        .func = &sau_sdk,
        .argtable = &sdk_args
    };

    const esp_console_cmd_t emergency_cmd = {
        .command = "emergency",
        .help = "emergency.\n"
        "Examples:\n"
        " emergency \n",
        .hint = NULL,
        .func = &sau_emergency,
        .argtable = &emergency_args
    };

    const esp_console_cmd_t channel_cmd = {
        .command = "channel",
        .help = "channel <channel> <value>, value 900~ 2100.\n"
        "Examples:\n"
        " channel 5 1400 \n",
        .hint = NULL,
        .func = &sau_channel,
        .argtable = &channel_args
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&switch_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&status_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&bluetooth_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&ap_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&wifi_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&sdk_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&emergency_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&channel_cmd));
}

