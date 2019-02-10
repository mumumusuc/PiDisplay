cmake_minimum_required(VERSION 2.8)
set(CMAKE_C_STANDARD 99)

project(pi_display)

set(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_C_FLAGS_RELEASE "$ENV{CFLAGS} -O3 -Wall")

message("Build type ${CMAKE_BUILD_TYPE}")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-DDEBUG)
	message("add definition DEBUG")
endif ()

include_directories(include)
include_directories(device/include)
include_directories(driver/include)
include_directories(graphic/include)

add_subdirectory(device)
add_subdirectory(driver)
add_subdirectory(graphic)
if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
add_subdirectory(kernel/gpio)
add_subdirectory(kernel/ssd1306)
add_subdirectory(kernel/hc_sr04)
endif()

aux_source_directory(${CMAKE_SOURCE_DIR} SRC_FILES)
link_libraries(device m)
add_executable(${PROJECT_NAME} ${SRC_FILES})

# link ffmpeg
find_library(LIB_FFMPEG NAMES avutil)
if (LIB_FFMPEG)
	message("Found ffmpeg, linking ...")
	add_definitions(-DFFMPEG)
	target_link_libraries(${PROJECT_NAME} avformat avdevice avcodec avutil x264 pthread z bz2 dl swscale)

	set(LIB_MMAL_PATH /opt/vc/lib)
	set(LIB_MMAL_LIBS mmal mmal_core mmal_vc_client mmal_components mmal_util)
	foreach (LIB ${LIB_MMAL_LIBS})
		find_library(LIB_MMAL_${LIB} ${LIB} ${LIB_MMAL_PATH})
		if (LIB_MMAL_${LIB})
			message("link -> ${LIB_MMAL_${LIB}}")
			target_link_libraries(${PROJECT_NAME} ${LIB_MMAL_${LIB}})
		endif ()
	endforeach ()
endif ()

# link FreeImage
find_library(LIB_FREE_IMAGE freeimage)
if (LIB_FREE_IMAGE)
	message("Found freeimage, linking ...")
	#add_definitions(-DFREE_IMAGE)
	target_link_libraries(${PROJECT_NAME} freeimage)
endif ()
