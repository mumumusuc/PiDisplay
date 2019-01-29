//
// Created by mumumusuc on 19-1-22.
//

#ifndef PI_DISPLAY_CANVAS_H
#define PI_DISPLAY_CANVAS_H

#include <stdlib.h>
#include <stdint.h>
#include "factory.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct _Canvas Canvas;

typedef void(*fpCanvasFlush)(Canvas *);

typedef void(*fpCanvasClear)(Canvas *, uint8_t);

typedef void *(*fpCanvasBind)(Canvas *, void *, size_t);

typedef void *(*fpCanvasUnbind)(Canvas *);

typedef enum {
    PIXEL_FMT_BIT = 1,
    PIXEL_FMT_8U1 = 8,
    PIXEL_FMT_8U3 = 24,
} PixelFormat;

typedef struct _Buffer {
    size_t width;
    size_t height;
    PixelFormat format;
    void *buffer;
} Buffer;

struct _Canvas {
    void *display_buffer;
    size_t size;
    Display *display;
    fpCanvasClear clear;
    fpCanvasFlush flush;
    fpCanvasBind bind;
    fpCanvasUnbind unbind;
};

Canvas *new_Canvas(void);

void del_Canvas(Canvas *);

#ifdef __cplusplus
}
#endif
#endif //PI_DISPLAY_CANVAS_H
