cmake_minimum_required(VERSION 2.8)
set(CMAKE_C_STANDARD 99)

project(device_ssd1306)

aux_source_directory(. SRC_FILES)
add_library(${PROJECT_NAME} SHARED ${SRC_FILES})

