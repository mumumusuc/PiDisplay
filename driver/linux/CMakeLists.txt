cmake_minimum_required(VERSION 2.8)
set(CMAKE_C_STANDARD 99)

project(driver_linux)
aux_source_directory(. SRC_FILES)

find_library(
        BCM_HOST_LIBRARIES
        NAMES bcm_host
        PATHS /usr/lib/ /usr/local/lib/ /opt/vc/lib/
)

if (BCM_HOST_LIBRARIES)
    link_libraries(${BCM_HOST_LIBRARIES})
endif ()

add_library(${PROJECT_NAME} SHARED ${SRC_FILES})

set(DRIVER_linux TRUE CACHE INTERNAL "Driver ${PROJECT_NAME} built.")