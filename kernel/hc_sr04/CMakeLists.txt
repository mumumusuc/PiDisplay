cmake_minimum_required(VERSION 2.8)

project(hc_sr04)
# setup gun-c99
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
# find linux kernel dir
exec_program(uname ARGS -r OUTPUT_VARIABLE CMAKE_HOST_SYSTEM_VERSION)
set(KERNEL_INCLUDE "/usr/src/linux-headers-${CMAKE_HOST_SYSTEM_VERSION}")
# include kernel dir
include_directories(
		"${KERNEL_INCLUDE}/include"
		"${KERNEL_INCLUDE}/mm"
)
add_definitions(-D__KERNEL__=1)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC_FILES)
add_executable(${PROJECT_NAME} ${SRC_FILES})
