idf_component_register(SRCS "cmd_udp.c"
                    INCLUDE_DIRS . ../../../ ../../../main ../../button/include
                    PRIV_REQUIRES driver esp_timer esp_event led_strip)

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
