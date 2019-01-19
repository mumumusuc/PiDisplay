//
// Created by mumumusuc on 19-1-19.
//

#ifndef PI_DISPLAY_DEVICE_H
#define PI_DISPLAY_DEVICE_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void(*fpInit)(void *);

typedef void(*fpReset)(void *);

typedef void(*fpTurnOn)(void *);

typedef void(*fpTurnOff)(void *);

// define base device
typedef struct _BaseDevice {
    fpInit init;
    fpReset reset;
    fpTurnOn turn_on;
    fpTurnOff turn_off;
} BaseDevice;

#ifdef __cplusplus
}
#endif
#endif //PI_DISPLAY_DEVICE_H
