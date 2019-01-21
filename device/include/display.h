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
typedef struct _DisplayInfo DisplayInfo;

typedef struct _BaseDisplay BaseDisplay;

typedef struct _DisplayOps DisplayOps;

typedef void(*fpInit)(BaseDisplay *);

typedef void(*fpReset)(BaseDisplay *);

typedef void(*fpTurnOn)(BaseDisplay *);

typedef void(*fpTurnOff)(BaseDisplay *);

typedef void(*fpUpdate)(BaseDisplay *, void *);

struct _DisplayInfo {
    char vendor[16];
    size_t width;
    size_t height;
    uint8_t pixel_format;
};

struct _DisplayOps {
    fpInit init;
    fpReset reset;
    fpTurnOn turn_on;
    fpTurnOff turn_off;
    fpUpdate update;
};

struct _BaseDisplay {
    DisplayInfo info;
    DisplayOps ops;
};

// constructor
BaseDisplay *new_Display();

void init_Display(BaseDisplay *, DisplayOps *, DisplayInfo *);

void delete_Display(BaseDisplay *);

#ifdef __cplusplus
}
#endif
#endif //PI_DISPLAY_DISPLAY_H
