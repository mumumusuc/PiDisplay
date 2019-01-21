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

Driver *new_Driver() {
    Driver *driver = (Driver *) malloc(sizeof(Driver));
    assert(driver);
    driver->init = init;
    driver->uninit = uninit;
    driver->write = write;
    driver->read = read;
    return driver;
}

void delete_Driver(Driver *driver) {
    free(driver);
    driver = NULL;
}