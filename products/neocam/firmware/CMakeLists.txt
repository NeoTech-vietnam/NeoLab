cmake_minimum_required(VERSION 3.16)
set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/BSW"
                         "${CMAKE_CURRENT_LIST_DIR}/Rte"
                         "${CMAKE_CURRENT_LIST_DIR}/ApplicationLayer")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
if(NOT IDF_VERSION_MAJOR EQUAL 6 OR NOT IDF_VERSION_MINOR EQUAL 0 OR NOT IDF_VERSION_PATCH EQUAL 1)
    message(FATAL_ERROR "NeoCam requires ESP-IDF 6.0.1; export that SDK before configuring")
endif()
project(xiao_esp32s3_camera_stream)
