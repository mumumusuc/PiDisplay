//
// Created by mumumusuc on 19-1-19.
//

#ifndef PI_DISPLAY_DISPLAY_H
#define PI_DISPLAY_DISPLAY_H

#include <stdint.h>
#include "device.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void(*fpGetInfo)(void *, struct DisplayInfo *);

typedef void(*fpUpdate)(void *, void *);

// define base display device
typedef struct _DisplayInfo {
    char *vendor;
    size_t width;
    size_t height;
    uint8_t pixel_format;
} DisplayInfo;

typedef struct _BaseDisplay {
    const BaseDevice *parent;
    fpGetInfo get_info;
    fpInit init;
    fpReset reset;
    fpTurnOn turn_on;
    fpTurnOff turn_off;
    fpUpdate update;
} BaseDisplay;

BaseDisplay *new_Display();

void delete_Display(BaseDisplay *display);

#ifdef __cplusplus
}
#endif
#endif //PI_DISPLAY_DISPLAY_H
