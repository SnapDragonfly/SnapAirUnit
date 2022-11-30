idf_component_register(SRCS "cmd_udp.c"
                    INCLUDE_DIRS . ../../../main)

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
