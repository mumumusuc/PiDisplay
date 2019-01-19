message(STATUS "Try to find bcm2835 ...")

find_path(
        BCM2835_INCLUDE_DIR
        bcm2835.h
        /usr/include/
        /usr/local/include/
)

find_library(
        BCM2835_LIBRARIES
        NAMES bcm2835
        PATHS /usr/lib/ /usr/local/lib/
)
if(BCM2835_INCLUDE_DIR AND BCM2835_LIBRARIES)
    message(STATUS "    include   -> ${BCM2835_INCLUDE_DIR}")
    message(STATUS "    libraries -> ${BCM2835_LIBRARIES}")
    set(BCM2835_FOUND TRUE)
endif()
