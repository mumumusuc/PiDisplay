cmake_minimum_required(VERSION 2.8)

project(ssd1306_spi4)
# find linux kernel dir
exec_program(uname ARGS -r OUTPUT_VARIABLE CMAKE_HOST_SYSTEM_VERSION)
message(${CMAKE_HOST_SYSTEM_VERSION})
set(KERNEL_INCLUDE "/usr/src/linux-headers-${CMAKE_HOST_SYSTEM_VERSION}")
# include kernel dir
include_directories(
		"${KERNEL_INCLUDE}/include"
		"${KERNEL_INCLUDE}/mm"
)
add_definitions(-D__KERNEL__=1)
add_definitions(-DKBUILD_MODNAME=1)

aux_source_directory(. SRC_FILES)
add_executable(${PROJECT_NAME} ${SRC_FILES})
