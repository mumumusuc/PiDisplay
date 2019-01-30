//
// Created by mumumusuc on 19-1-22.
//

#include <assert.h>
#include <string.h>

#include "common.h"
#include "include/canvas.h"

#define LOG_TAG "Canvas"

//int MODEL = DISPLAY_SSD1306_BCM_SPI4;
/*
static void bind_display(Canvas *canvas) {
    assert(!canvas->display);
    LOG("bind_display");
#ifdef TEST
    Display *disp = create_display(DISPLAY_NONE);
#else
    Display *disp = create_display(MODEL);
#endif
    size_t size = (disp->info.width) * (disp->info.height) * (disp->info.pixel_format) / 8;
    canvas->display_buffer = calloc(1, size);
    canvas->size = size;
    canvas->display = disp;
    disp->ops.begin(disp);
    disp->ops.reset(disp);
    disp->ops.turn_on(disp);
}

static void clear(Canvas *canvas, uint8_t value) {
    LOG("clear");
    if (canvas->display_buffer && canvas->size) {
        memset(canvas->display_buffer, value, canvas->size);
    }
}

static void flush(Canvas *canvas) {
    //LOG("flush");
    if (!canvas->display) {
        bind_display(canvas);
    }
    Display *disp = canvas->display;
    disp->ops.update(disp, canvas->display_buffer);
}

static void *bind(Canvas *canvas, void *buffer, size_t size) {
    LOG("bind");
    assert(buffer && size);
    void *stock = canvas->display_buffer;
    canvas->display_buffer = buffer;
    canvas->size = size;
    return stock;
}

static void *unbind(Canvas *canvas) {
    LOG("unbind");
    void *buffer = canvas->display_buffer;
    canvas->size = 0;
    return buffer;
}


Canvas *new_Canvas(int opt) {
    if (opt) MODEL = DISPLAY_SSD1306_BCM_I2C;
    Canvas *canvas = (Canvas *) malloc(sizeof(Canvas));
    assert(canvas);
    canvas->clear = clear;
    canvas->bind = bind;
    canvas->unbind = unbind;
    canvas->flush = flush;
    canvas->display_buffer = NULL;
    canvas->size = 0;
    canvas->display = NULL;
    // bind_display
    bind_display(canvas);
    // bind buffer
    DisplayInfo info = canvas->display->info;
    size_t size = info.width * info.height * info.pixel_format / 8;
    void *buffer = calloc(1, size);
    assert(buffer);
    bind(canvas, buffer, size);

    return canvas;
}

void del_Canvas(Canvas *canvas) {
    if (canvas) {
        free(canvas->display_buffer);
        canvas->display_buffer = NULL;
        if (canvas->display) {
            Display *disp = canvas->display;
            //disp->ops.turn_off(disp);
            disp->ops.end(disp);
            destroy_display(canvas->display);
        }
    }
    free(canvas);
    canvas = NULL;
}*/