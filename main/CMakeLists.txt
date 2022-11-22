idf_component_register(SRCS "main.c"
                            "softap.c"
                            "station.c"
                            "btspp.c"
                            "process.c"
                            "blink.c"
                            "key.c"
                            "mode.c"
                            "util.c"
                            "ttl.c"
                            "module.c"
                       INCLUDE_DIRS ".")
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")