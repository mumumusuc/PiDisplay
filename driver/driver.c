//
// Created by mumumusuc on 19-1-19.
//

#include <assert.h>
#include "common.h"
#include "driver.h"

#define LOG_TAG     "DRIVER"

static void init(void *self, void *_) {
#ifdef DEBUG
    LOG("init");
#else
    METHOD_NOT_IMPLEMENTED("init");
#endif
}

static void uninit(void *self) {
#ifdef DEBUG
    LOG("uninit");
#else
    METHOD_NOT_IMPLEMENTED("uninit");
#endif
}

static int dwrite(void *self, uint8_t *data, size_t size) {
#ifdef DEBUG
    LOG("write");
#else
    METHOD_NOT_IMPLEMENTED("write");
#endif
}

static int dread(void *self, uint8_t *buffer, size_t size) {
#ifdef DEBUG
    LOG("read");
#else
    METHOD_NOT_IMPLEMENTED("read");
#endif
}

static void init_Driver(void *restrict _driver, DriverOps *restrict ops) {
    assert(_driver && ops);
    DriverOps *driver = (DriverOps *) _driver;
    driver->init = ops->init;
    driver->uninit = ops->uninit;
    driver->read = ops->read;
    driver->write = ops->write;
}

static void *new_Driver(size_t size) {
    void *driver = malloc(size);
    DriverOps ops = {
            .init = init,
            .uninit = uninit,
            .read = dread,
            .write = dwrite,
    };
    init_Driver(driver, &ops);
    return driver;
}

static void del_Driver(void *driver) {
    free(driver);
    driver = NULL;
}

// implements
GPIO *new_GPIO() {
    return (GPIO *) new_Driver(sizeof(GPIO));
}

void del_GPIO(GPIO *gpio) {
    del_Driver(gpio);
}

void init_GPIO(GPIO *gpio, DriverOps *ops) {
    init_Driver(gpio, ops);
}

I2C *new_I2C() {
    return (I2C *) new_Driver(sizeof(I2C));
}

void del_I2C(I2C *i2c) {
    del_Driver(i2c);
}

void init_I2C(I2C *i2c, DriverOps *ops) {
    init_Driver(i2c, ops);
}

SPI *new_SPI() {
    return (SPI *) new_Driver(sizeof(SPI));
}

void del_SPI(SPI *spi) {
    del_Driver(spi);
}

void init_SPI(SPI *spi, DriverOps *ops) {
    init_Driver(spi, ops);
}