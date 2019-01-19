//
// Created by mumumusuc on 19-1-19.
//

#include <assert.h>
#include "display.h"


// implement base device methods
static void get_info(void *self, DisplayInfo *info) {
    METHOD_NOT_IMPLEMENTED("get_info");
}

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
    BaseDisplay *display = NULL;
    display = (BaseDisplay *) calloc(1, sizeof(BaseDisplay));
    assert(display);
    display->parent = (BaseDevice *) calloc(1, sizeof(BaseDevice));
    assert(display->parent);
    display->get_info = get_info;
    display->init = init;
    display->reset = reset;
    display->turn_on = turn_on;
    display->turn_off = turn_off;
    display->update = update;
}

void delete_Display(BaseDisplay *display) {
    if (display) {
        free(display->parent);
        display->parent = NULL;
    }
    free(display);
    display = NULL;
}


