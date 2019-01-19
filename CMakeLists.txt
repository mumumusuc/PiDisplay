cmake_minimum_required(VERSION 2.8)
project(pi_display)
set(CMAKE_C_STANDARD 99)

aux_source_directory(. SRC_FILES)

add_executable(${PROJECT_NAME} ${SRC_FILES})