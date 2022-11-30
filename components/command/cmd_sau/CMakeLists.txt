idf_component_register(SRCS "cmd_sau.c"
                    INCLUDE_DIRS . ../../../main ../cmd_udp ../../..
                    REQUIRES console)

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
