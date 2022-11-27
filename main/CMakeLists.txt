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
                            "rest_server.c"
                            "udp_server.c"
                            "udp_client.c"
                       INCLUDE_DIRS ".")

if(CONFIG_RESTFUL_WEB_DEPLOY_SF)
    set(WEB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../front/web-demo")
    if(EXISTS ${WEB_SRC_DIR}/dist)
        spiffs_create_partition_image(www ${WEB_SRC_DIR}/dist FLASH_IN_PROJECT)
    else()
        message(FATAL_ERROR "${WEB_SRC_DIR}/dist doesn't exit. Please run 'npm run build' in ${WEB_SRC_DIR}")
    endif()
endif()

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")