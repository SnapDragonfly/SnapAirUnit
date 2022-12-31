/// @file cmd_sau.c

/*
 * idf header files
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_console.h"
#include "esp_err.h"
#include "argtable3/argtable3.h"

/*
 * basic header files
 */
#include "define.h"

/*
 * module header files
 */
#include "module.h"
#include "cmd_sau.h"
#include "cmd_udp.h"
#include "msp_protocol.h"
#include "mode.h"
#include "util.h"
#include "factory_setting.h"

/*
 * service header files
 */
//TBD.

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

static struct {
    struct arg_end *end;
} reboot_args;

static struct {
    struct arg_str *mode;
    struct arg_end *end;
} command_args;

static struct {
    struct arg_str *confirm;
    struct arg_end *end;
} restore_args;


static int sau_switch(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &mode_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, mode_args.end, argv[0]);
        return 1;
    }

    enum_wireless_mode_t mode = (enum_wireless_mode_t)strtol(mode_args.mode->sval[0], NULL, 0);

    esp_err_t err = wireless_mode_switch(mode);

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

    enum_protocol_state_t state  = protocol_state_get();
    enum_wireless_mode_t mode   = wireless_mode_get();

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

    ESP_LOGI(MODULE_CMD_SAU, "SDK Version: %s", get_idf_versions());
    ESP_LOGI(MODULE_CMD_SAU, "APP Version: %s", get_app_versions());
    ESP_LOGI(MODULE_CMD_SAU, "%s: current free_heap_size = %d", DEVICE_NAME_SNAP_AIR_UNIT, esp_get_free_heap_size());
    ESP_LOGI(MODULE_CMD_SAU, "%s: minimum free_heap_size = %d", DEVICE_NAME_SNAP_AIR_UNIT, esp_get_minimum_free_heap_size());
    return 0;
}

static int sau_emergency(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &emergency_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, emergency_args.end, argv[0]);
        return 1;
    }

    auc_set_channel(4, 1200);
    auc_update_channels();

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

    auc_set_channel(channel, value);
    auc_update_channels();

    ESP_LOGI(MODULE_CMD_SAU, "channel %d %d", channel, value);
    return 0;
}

static int sau_reboot(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &reboot_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, reboot_args.end, argv[0]);
        return 1;
    }

    (void)UTIL_reboot(3);
    return 0;
}

static int sau_command(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &command_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, command_args.end, argv[0]);
        return 1;
    }

    int mode = strtol(command_args.mode->sval[0], NULL, 0);

    return snap_sw_command_set(mode);
}

static int sau_restore(int argc, char **argv)
{
    esp_err_t err = ESP_FAIL;

    int nerrors = arg_parse(argc, argv, (void **) &restore_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, restore_args.end, argv[0]);
        return 1;
    }

    const char *confirm = restore_args.confirm->sval[0];
    if(!strcmp(CONSOLE_PROMPT_YES, confirm)){
        err = restore_factory_settings();
    } else {
        ESP_LOGI(MODULE_CMD_SAU, "Skip restore settings.");
    }

    return err;
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

    reboot_args.end        = arg_end(2);

    command_args.mode      = arg_str1(NULL, NULL, "<mode>", "command mode");
    command_args.end       = arg_end(2);

    restore_args.confirm   = arg_str1(NULL, NULL, "<confirm>", "confirmation");
    restore_args.end       = arg_end(2);

    const esp_console_cmd_t switch_cmd = {
        .command = "switch",
        .help = "switch <mode>, mode:ap(0)/sta(1)/bt(2).\n"
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
        .help = "status, get status of the application.\n"
        "Examples:\n"
        " status \n",
        .hint = NULL,
        .func = &sau_status,
        .argtable = &status_args
    };

    const esp_console_cmd_t bluetooth_cmd = {
        .command = "bluetooth",
        .help = "bluetooth, switch to BT SPP UART.\n"
        "Examples:\n"
        " bluetooth \n",
        .hint = NULL,
        .func = &sau_bluetooth,
        .argtable = &bluetooth_args
    };

    const esp_console_cmd_t ap_cmd = {
        .command = "ap",
        .help = "ap <ssid> <pass>, switch to AP.\n"
        "Examples:\n"
        " ap SnapAirUnit 12345678 \n",
        .hint = NULL,
        .func = &sau_ap,
        .argtable = &wifi_ap_args
    };

    const esp_console_cmd_t sta_cmd = {
        .command = "sta",
        .help = "sta <ssid> <pass>, connect to AP.\n"
        "Examples:\n"
        " sta SnapAirUnit 12345678 \n",
        .hint = NULL,
        .func = &sau_wifi,
        .argtable = &wifi_sta_args
    };

    const esp_console_cmd_t sdk_cmd = {
        .command = "sdk?",
        .help = "sdk?, get application version.\n"
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

    const esp_console_cmd_t reboot_cmd = {
        .command = "reboot",
        .help = "reboot.\n"
        "Examples:\n"
        " reboot \n",
        .hint = NULL,
        .func = &sau_reboot,
        .argtable = &reboot_args
    };

    const esp_console_cmd_t command_cmd = {
        .command = "command",
        .help = "command <mode>, mode:false(0)/true(1).\n"
        "Examples:\n"
        " command 0 \n"
        " command 1 \n",
        .hint = NULL,
        .func = &sau_command,
        .argtable = &command_args
    };

    const esp_console_cmd_t restore_cmd = {
        .command = "restore",
        .help = "restore <confirm>, confirm: yes or no.\n"
        "Examples:\n"
        " restore yes\n",
        .hint = NULL,
        .func = &sau_restore,
        .argtable = &restore_args
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&switch_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&status_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&bluetooth_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&ap_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&sta_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&sdk_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&emergency_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&channel_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&reboot_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&command_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&restore_cmd));
}

