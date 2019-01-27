//
// Created by mumumusuc on 19-1-25.
//

#ifndef PI_DISPLAY_DISPLAY_PROTECTED_H
#define PI_DISPLAY_DISPLAY_PROTECTED_H

#include "display_.h"

#ifdef __cplusplus
extern "C" {
#endif

#define override(disp, impl) do{assert((disp) && (impl));(disp)->vtbl = (impl);}while(0)

typedef void(*fpDspBegin)(Display *);

typedef void(*fpDspReset)(Display *);

typedef void(*fpDspTurnOn)(Display *);

typedef void(*fpDspClear)(Display *);

typedef void(*fpDspUpdate)(Display *, const void *);

typedef void(*fpDspTurnOff)(Display *);

typedef void(*fpDspEnd)(Display *);

struct _Display_VTbl {
    fpDspBegin begin;
    fpDspReset reset;
    fpDspTurnOn turn_on;
    fpDspClear clear;
    fpDspUpdate update;
    fpDspTurnOff turn_off;
    fpDspEnd end;
};

#ifdef __cplusplus
}
#endif

#endif //PI_DISPLAY_DISPLAY_PROTECTED_H