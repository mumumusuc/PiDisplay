cmake_minimum_required(VERSION 2.8)
set(CMAKE_C_STANDARD 99)

project(pi_display)

add_subdirectory(device)
add_subdirectory(driver)

aux_source_directory(. SRC_FILES)
add_executable(${PROJECT_NAME} ${SRC_FILES})