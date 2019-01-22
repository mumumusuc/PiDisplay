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

typedef struct _Display Display;

typedef struct _DisplayOps DisplayOps;

typedef void(*fpDspBegin)(Display *);

typedef void(*fpDspReset)(Display *);

typedef void(*fpDspTurnOn)(Display *);

typedef void(*fpDspClear)(Display *);

typedef void(*fpDspUpdate)(Display *, void *);

typedef void(*fpDspTurnOff)(Display *);

typedef void(*fpDspEnd)(Display *);

struct _DisplayInfo {
    char vendor[16];
    size_t width;
    size_t height;
    uint8_t pixel_format;
};

// TODO: This should be private
struct _DisplayOps {
    fpDspBegin begin;
    fpDspReset reset;
    fpDspTurnOn turn_on;
    fpDspClear clear;
    fpDspUpdate update;
    fpDspTurnOff turn_off;
    fpDspEnd end;
};

struct _Display {
    DisplayInfo info;
    DisplayOps ops;
};

// TODO: These should be protected
void init_Display(Display *, DisplayOps *, DisplayInfo *);

// constructor & destructor
Display *new_Display();

void del_Display(Display *);
// end constructor

#ifdef __cplusplus
}
#endif
#endif //PI_DISPLAY_DISPLAY_H
