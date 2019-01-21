cmake_minimum_required(VERSION 2.8)
set(CMAKE_C_STANDARD 99)

project(device)
message("Make display device libs.")

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC_FILES)
add_library(${PROJECT_NAME} SHARED ${SRC_FILES})

file(GLOB DEVICES_LIST RELATIVE ${CMAKE_CURRENT_LIST_DIR} "*")
list(REMOVE_ITEM DEVICES_LIST "include" "display.c" "CMakeLists.txt")
foreach (DEVICE ${DEVICES_LIST})
    message(STATUS "Find device: ${DEVICE}")
    add_subdirectory(${DEVICE})
    target_link_libraries(${PROJECT_NAME} device_${DEVICE})
endforeach ()