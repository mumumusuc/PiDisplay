//
// Created by mumumusuc on 19-1-19.
//

#include <assert.h>
#include "common.h"
#include "driver.h"

static void init(void *_) {
    METHOD_NOT_IMPLEMENTED("init");
}

static void uninit() {
    METHOD_NOT_IMPLEMENTED("uninit");
}

static int write(uint8_t *data, size_t size) {
    METHOD_NOT_IMPLEMENTED("write");
}

static int read(uint8_t *buffer, size_t size) {
    METHOD_NOT_IMPLEMENTED("read");
}

void init_Driver(Driver *restrict dst, Driver *restrict src) {
    assert(dst || src);
    dst->init = src->init;
    dst->uninit = src->uninit;
    dst->read = src->read;
    dst->write = src->write;
}