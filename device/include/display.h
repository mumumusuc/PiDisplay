//
// Created by mumumusuc on 19-1-25.
//

#ifndef PI_DISPLAY_DISPLAY_H
#define PI_DISPLAY_DISPLAY_H

#include <stdint.h>
#include <stdlib.h>
#include "object.h"

#ifdef __cplusplus
extern "C" {
#endif

#define API_OVERLOAD __attribute__((overloadable))

typedef struct _DisplayInfo DisplayInfo;

typedef struct _Display Display;

typedef struct _DisplayOps DisplayOps;

typedef struct _Display_VTbl DisplayVTbl;

struct _DisplayInfo {
    char vendor[16];
    size_t width;
    size_t height;
    uint8_t pixel_format;
};

struct _Display {
    DisplayVTbl *vtbl;
    Object *obj;
};

// define methods
Display *new_display(void);

void display_get_info(Display *, DisplayInfo *);

void display_begin(Display *);

void display_reset(Display *);

void display_turn_on(Display *);

void display_clear(Display *);

void display_update(Display *, const void *);

void display_turn_off(Display *);

void display_end(Display *);
// end define methods

#ifdef __cplusplus
}
#endif

#endif //PI_DISPLAY_DISPLAY_H
