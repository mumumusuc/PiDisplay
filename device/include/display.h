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

typedef void(*fpDestroy)(void *);

struct _DisplayInfo {
    char vendor[16];
    size_t width;
    size_t height;
    uint8_t pixel_format;
};

struct _DisplayOps {
    fpDspBegin begin;
    fpDspReset reset;
    fpDspTurnOn turn_on;
    fpDspClear clear;
    fpDspUpdate update;
    fpDspTurnOff turn_off;
    fpDspEnd end;
};

typedef struct _VTbl VTbl;

struct _Display {
    DisplayInfo info;
    DisplayOps ops;
    fpDestroy destructor;
    void *this;
};


#ifdef __cplusplus
}
#endif
#endif //PI_DISPLAY_DISPLAY_H
