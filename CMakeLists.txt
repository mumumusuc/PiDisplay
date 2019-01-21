cmake_minimum_required(VERSION 2.8)
set(CMAKE_C_STANDARD 99)

project(pi_display)

include_directories(include)
include_directories(device/include)
include_directories(driver/include)

add_subdirectory(device)
add_subdirectory(driver)

aux_source_directory(. SRC_FILES)
link_libraries(device driver)
add_executable(${PROJECT_NAME} ${SRC_FILES})
