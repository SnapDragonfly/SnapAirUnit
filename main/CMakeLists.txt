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
                            "spiffs.c"
                            "tello_protocol.c"
                            "hy_protocol.c"
                            "msp_protocol.c"
                            "console.c"
                            "factory_setting.c"
                            "handle.c"
                       INCLUDE_DIRS ".")

if(CONFIG_RESTFUL_WEB_DEPLOY_SF)
    set(WEB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../front/web-demo")
    if(EXISTS ${WEB_SRC_DIR}/dist)
        spiffs_create_partition_image(www ${WEB_SRC_DIR}/dist FLASH_IN_PROJECT)
    else()
        message(FATAL_ERROR "${WEB_SRC_DIR}/dist doesn't exit. Please run 'npm run build' in ${WEB_SRC_DIR}")
    endif()
endif()

set(STORAGE_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../build/storage")
if(EXISTS ${STORAGE_SRC_DIR})
	spiffs_create_partition_image(storage ${STORAGE_SRC_DIR} FLASH_IN_PROJECT)
else()
	message(FATAL_ERROR "${STORAGE_SRC_DIR} doesn't exit. Please check in ${STORAGE_SRC_DIR}")
endif()

set(NVS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../build/nvs")
if(EXISTS ${NVS_SRC_DIR})
	spiffs_create_partition_image(nvs ${NVS_SRC_DIR} FLASH_IN_PROJECT)
else()
	message(FATAL_ERROR "${NVS_SRC_DIR} doesn't exit. Please check in ${NVS_SRC_DIR}")
endif()

set(PHY_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../build/phy_init")
if(EXISTS ${PHY_SRC_DIR})
	spiffs_create_partition_image(phy_init ${PHY_SRC_DIR} FLASH_IN_PROJECT)
else()
	message(FATAL_ERROR "${PHY_SRC_DIR} doesn't exit. Please check in ${PHY_SRC_DIR}")
endif()

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")