cmake_minimum_required(VERSION 2.8)
set(CMAKE_C_STANDARD 99)

project(driver)
message("Make display driver libs.")

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC_FILES)
add_library(${PROJECT_NAME} SHARED ${SRC_FILES})

file(GLOB DRIVER_LIST RELATIVE ${CMAKE_CURRENT_LIST_DIR} "*")
list(REMOVE_ITEM DRIVER_LIST "include" "driver.c" "CMakeLists.txt")
foreach (DRIVER ${DRIVER_LIST})
    message(STATUS "Find driver: ${DRIVER}.")
    add_subdirectory(${DRIVER})
    if(${DRIVER_${DRIVER}})
        message(STATUS "---- link driver ${DRIVER}")
        target_link_libraries(${PROJECT_NAME} driver_${DRIVER})
    endif()
endforeach ()