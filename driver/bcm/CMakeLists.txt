cmake_minimum_required(VERSION 2.8)
set(CMAKE_C_STANDARD 99)

project(driver_bcm)
aux_source_directory(. SRC_FILES)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
find_package(bcm2835)

if (BCM2835_FOUND)
    include_directories(${BCM2835_INCLUDE_DIR})
    link_libraries(${BCM2835_LIBRARIES})
    aux_source_directory(. SRC_FILES)
    add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
    set(DRIVER_bcm TRUE CACHE INTERNAL "Driver ${PROJECT_NAME} built.")
else ()
    message(WARNING "Not found bcm2835, skip this build.")
    set(DRIVER_bcm2835 FALSE CACHE INTERNAL "Driver ${PROJECT_NAME} not built.")
endif ()