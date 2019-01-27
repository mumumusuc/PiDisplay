cmake_minimum_required(VERSION 2.8)
set(CMAKE_C_STANDARD 99)

project(graphic)
message("Make display graphic libs.")

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC_FILES)
link_libraries(device)
add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
