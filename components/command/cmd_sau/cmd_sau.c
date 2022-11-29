

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

#include "module.h"
#include "mode.h"

static struct {
    struct arg_str *mode;
    struct arg_end *end;
} mode_args;

static struct {
    struct arg_end *end;
} status_args;


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
        ESP_LOGE(MODULE_SAU, "%s", esp_err_to_name(err));
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

    ESP_LOGI(MODULE_SAU, "Application mode(%d) state(%d)", mode, state);
    ESP_LOGI(MODULE_SAU, "Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

    return 0;
}


void register_sau(void)
{
    mode_args.mode = arg_str1(NULL, NULL, "<mode>", "application mode to be set");
    mode_args.end  = arg_end(2);

    status_args.end  = arg_end(2);

    const esp_console_cmd_t switch_cmd = {
        .command = "switch",
        .help = "Switch application mode to ap(0)/sta(1)/bt(2).\n"
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

    ESP_ERROR_CHECK(esp_console_cmd_register(&switch_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&status_cmd));
}
