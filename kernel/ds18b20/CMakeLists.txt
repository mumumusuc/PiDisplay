cmake_minimum_required(VERSION 2.8)

project(ds18b20)
# setup gun-c99
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC_FILES)
add_executable(${PROJECT_NAME} ${SRC_FILES})
