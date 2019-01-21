//
// Created by mumumusuc on 19-1-21.
//

#ifndef PI_DISPLAY_DISPLAY_H
#define PI_DISPLAY_DISPLAY_H

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void(*fpInit)(struct _BaseDisplay *);

typedef void(*fpReset)(struct _BaseDisplay *);

typedef void(*fpTurnOn)(struct _BaseDisplay *);

typedef void(*fpTurnOff)(struct _BaseDisplay *);

typedef void(*fpUpdate)(struct _BaseDisplay *, void *);

typedef struct _DisplayOps {
    fpInit init;
    fpReset reset;
    fpTurnOn turn_on;
    fpTurnOff turn_off;
    fpUpdate update;
} DisplayOps;

typedef struct _BaseDisplay {
    char vendor[16];
    size_t width;
    size_t height;
    uint8_t pixel_format;
    DisplayOps *ops;
} BaseDisplay;

// constructor
BaseDisplay *new_Display();

void init_Display(BaseDisplay *, DisplayOps *);

void delete_Display(BaseDisplay *);

#ifdef __cplusplus
}
#endif
#endif //PI_DISPLAY_DISPLAY_H
