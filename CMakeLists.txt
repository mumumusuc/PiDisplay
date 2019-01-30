cmake_minimum_required(VERSION 2.8)
set(CMAKE_C_STANDARD 99)

project(pi_display)


set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_C_FLAGS_RELEASE "$ENV{CFLAGS} -O3 -Wall")

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    ADD_DEFINITIONS(-DDEBUG)
    message("add definition DEBUG")
endif ()

include_directories(include)
include_directories(device/include)
include_directories(driver/include)
include_directories(graphic/include)

add_subdirectory(device)
add_subdirectory(driver)
add_subdirectory(graphic)

aux_source_directory(${CMAKE_SOURCE_DIR} SRC_FILES)
link_libraries(device)
add_executable(${PROJECT_NAME} ${SRC_FILES})

set(ffmpeg avformat avcodec avutil swscale avdevice)

find_package(OpenCV)
message("Found OpenCV: ${OpenCV_FOUND}")
if (${OpenCV_FOUND})
    include_directories(${OpenCV_INCLUDE_DIRS})
    link_directories(${OpenCV_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
endif ()

target_link_libraries(${PROJECT_NAME} ${ffmpeg} m)

