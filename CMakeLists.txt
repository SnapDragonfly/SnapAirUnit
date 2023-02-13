# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

set(APP_VER_MAJOR "0")
set(APP_VER_MINOR "3")

set(EXTRA_COMPONENT_DIRS "components/button" 
                         "components/command/cmd_udp"
                         "components/command/cmd_sau"
                         "components/command/cmd_nvs")

macro(Get_COMMITID APP_COMMITID)
    find_package(Git QUIET)
    if(GIT_FOUND)
      execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        OUTPUT_VARIABLE ${APP_COMMITID}
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endif()
endmacro()
 
macro(Get_DIRTYFLAG APP_DIRTYFLAG)
    find_package(Git QUIET)
    if(GIT_FOUND)
      execute_process(
        COMMAND ./checkver.sh
        OUTPUT_VARIABLE ${APP_DIRTYFLAG}
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endif()
endmacro()

macro(Get_DIRPART APP_PARTCHECK)
    find_package(Git QUIET)
    if(GIT_FOUND)
      execute_process(
        COMMAND ./checkpart.sh
        OUTPUT_VARIABLE ${APP_PARTCHECK}
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endif()
endmacro()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(APP_COMMITID "")
Get_CommitID(APP_COMMITID)

set(APP_PARTCHECK "")
Get_DIRPART(APP_PARTCHECK)

set(APP_DIRTYFLAG "")
Get_DIRTYFLAG(APP_DIRTYFLAG)

add_definitions(-DAPP_VERSION=\"${APP_VER_MAJOR}.${APP_VER_MINOR}.${APP_COMMITID}-${APP_DIRTYFLAG}\")

project(SnapAirUnit)

message( STATUS "Code: ${APP_DIRTYFLAG}" )



