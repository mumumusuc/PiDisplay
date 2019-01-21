//
// Created by mumumusuc on 19-1-19.
//

#include <assert.h>
#include <string.h>
#include "common.h"
#include "include/display.h"


// implement base device methods
static void init(void *self) {
    METHOD_NOT_IMPLEMENTED("init");
}

static void reset(void *self) {
    METHOD_NOT_IMPLEMENTED("reset");
}

static void turn_on(void *self) {
    METHOD_NOT_IMPLEMENTED("turn_on");
}

static void turn_off(void *self) {
    METHOD_NOT_IMPLEMENTED("turn_off");
}

static void update(void *self, void *buffer) {
    METHOD_NOT_IMPLEMENTED("update");
}

// constructor & destructor
BaseDisplay *new_Display() {
    BaseDisplay *display = (BaseDisplay *) malloc(sizeof(BaseDisplay));
    assert(display);
    DisplayInfo info = {
            .width = 0,
            .height = 0,
            .pixel_format = 0,
            .vendor = "BaseDisplay",
    };
    DisplayOps ops = {
            .init = init,
            .reset = reset,
            .turn_on = turn_on,
            .turn_off = turn_off,
            .update = update,
    };
    init_Display(display, &ops, &info);
    return display;
}

void delete_Display(BaseDisplay *display) {
    free(display);
    display = NULL;
}


void init_Display(BaseDisplay *display, DisplayOps *ops, DisplayInfo *info) {
    assert(display && ops && info);
    strcpy(display->info.vendor, info->vendor);
    display->info.width = info->width;
    display->info.height = info->height;
    display->info.pixel_format = info->pixel_format;
    DisplayOps *ptrOps = &(display->ops);
    ptrOps->init = ops->init;
    ptrOps->reset = ops->reset;
    ptrOps->turn_on = ops->turn_on;
    ptrOps->turn_off = ops->turn_off;
    ptrOps->update = ops->update;
    ptrOps = NULL;
}


