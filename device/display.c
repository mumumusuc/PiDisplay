//
// Created by mumumusuc on 19-1-19.
//

#include <assert.h>
#include <string.h>
#include "common.h"
#include "include/display.h"

// default methods
static void begin(Display *self) {
    METHOD_NOT_IMPLEMENTED("begin");
}

static void reset(Display *self) {
    METHOD_NOT_IMPLEMENTED("reset");
}

static void turn_on(Display *self) {
    METHOD_NOT_IMPLEMENTED("turn_on");
}

static void clear(Display *self) {
    METHOD_NOT_IMPLEMENTED("clear");
}

static void update(Display *self, void *buffer) {
    METHOD_NOT_IMPLEMENTED("update");
}

static void turn_off(Display *self) {
    METHOD_NOT_IMPLEMENTED("turn_off");
}


static void end(Display *self) {
    METHOD_NOT_IMPLEMENTED("end");
}


void init_Display(Display *display, DisplayOps *ops, DisplayInfo *info) {
    assert(display && ops && info);
    // init struct DisplayInfo
    strcpy(display->info.vendor, info->vendor);
    display->info.width = info->width;
    display->info.height = info->height;
    display->info.pixel_format = info->pixel_format;
    // init struct DisplayOps
    DisplayOps *ptrOps = &(display->ops);
    ptrOps->begin = ops->begin;
    ptrOps->reset = ops->reset;
    ptrOps->turn_on = ops->turn_on;
    ptrOps->clear = ops->clear;
    ptrOps->update = ops->update;
    ptrOps->turn_off = ops->turn_off;
    ptrOps->end = ops->end;
    ptrOps = NULL;
}

// constructor & destructor
Display *new_Display() {
    Display *display = (Display *) malloc(sizeof(Display));
    assert(display);
    DisplayInfo info = {
            .width = 0,
            .height = 0,
            .pixel_format = 0,
            .vendor = "BaseDisplay",
    };
    DisplayOps ops = {
            .begin = begin,
            .reset = reset,
            .turn_on = turn_on,
            .clear = clear,
            .update = update,
            .turn_off = turn_off,
            .end = end,
    };
    init_Display(display, &ops, &info);
    return display;
}

void del_Display(Display *display) {
    free(display);
    display = NULL;
}


