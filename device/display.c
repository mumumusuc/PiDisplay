//
// Created by mumumusuc on 19-1-25.
//

#include <assert.h>
#include <string.h>
#include "common.h"
#include "display_protected.h"

#undef LOG_TAG
#define LOG_TAG "DISPLAY"

// default methods
static DisplayInfo _info = {
        .width = 0,
        .height = 0,
        .pixel_format = 0,
        .vendor="VirtualDisplay",
};

static void _get_info(Display *self, DisplayInfo *info) {
    if (info) {
        memcpy(info, &_info, sizeof(DisplayInfo));
    }
}

static void _begin(Display *self) {
    DEFAULT_METHOD();
}

static void _reset(Display *self) {
    DEFAULT_METHOD();
}

static void _turn_on(Display *self) {
    DEFAULT_METHOD();
}

static void _clear(Display *self) {
    DEFAULT_METHOD();
}

static void _update(Display *self, const void *buffer) {
    DEFAULT_METHOD();
}

static void _turn_off(Display *self) {
    DEFAULT_METHOD();
}

static void _end(Display *self) {
    DEFAULT_METHOD();
}
// end default methods

static DisplayVTbl _vtbl = {
        .get_info = _get_info,
        .begin = _begin,
        .reset = _reset,
        .turn_on = _turn_on,
        .clear = _clear,
        .update = _update,
        .turn_off = _turn_off,
        .end = _end,
};

inline Display *new_display(void) {
    Display *display = (Display *) malloc(sizeof(Display));
    assert(display);
    object_link(&(display->obj), NULL, display, (void *) del_display, "DISPLAY");
    override(display, &_vtbl);
    return display;
}

inline void del_display(Display *dsp) {
    if (dsp) {
        object_delete(dsp->obj);
    }
    free(dsp);
    dsp = NULL;
}

inline void display_get_info(Display *dsp, DisplayInfo *info) {
    eval_vtbl(dsp, get_info, info);
}

inline void display_begin(Display *dsp) {
    eval_vtbl(dsp, begin);
}

inline void display_reset(Display *dsp) {
    eval_vtbl(dsp, reset);
}

inline void display_turn_on(Display *dsp) {
    eval_vtbl(dsp, turn_on);
}

inline void display_clear(Display *dsp) {
    eval_vtbl(dsp, turn_on);
}

inline void display_update(Display *dsp, const void *buffer) {
    eval_vtbl(dsp, update, buffer);
}

inline void display_turn_off(Display *dsp) {
    eval_vtbl(dsp, turn_off);
}

inline void display_end(Display *dsp) {
    eval_vtbl(dsp, end);
}