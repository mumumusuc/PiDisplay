//
// Created by mumumusuc on 19-1-25.
//

#include <assert.h>
#include "common.h"
#include "driver_protected.h"

#undef  LOG_TAG
#define LOG_TAG "DRIVER"

// define gpio
static void _gpio_begin(Gpio *driver) {
    DEFAULT_METHOD();
}

static void _gpio_init(Gpio *driver, const GpioInfo *info) {
    DEFAULT_METHOD();
}

static void _gpio_write(Gpio *driver, const uint8_t *buffer, size_t size) {
    DEFAULT_METHOD();
}

static void _gpio_read(Gpio *driver, uint8_t *buffer, size_t size) {
    DEFAULT_METHOD();
}

static void _gpio_end(Gpio *driver) {
    DEFAULT_METHOD();
}

static GpioVTbl _gpio_vtbl = {
        .begin = _gpio_begin,
        .init = _gpio_init,
        .write = _gpio_write,
        .read = _gpio_read,
        .end = _gpio_end,
};

inline Gpio *new_gpio(void) {
    Gpio *gpio = (Gpio *) malloc(sizeof(Gpio));
    assert(gpio);
    link(gpio, "GPIO", del_gpio);
    gpio->vtbl = &_gpio_vtbl;
    return gpio;
}

inline void del_gpio(void *self) {
    if (self) {
        Gpio *_self = (Gpio *) self;
        object_delete(_self->obj);
    }
    free(self);
    self = NULL;
}

inline void gpio_begin(Gpio *self) {
    eval_vtbl(self, begin);
}

inline void gpio_init(Gpio *self, const GpioInfo *info) {
    eval_vtbl(self, init, info);
}

inline void gpio_write(Gpio *self, const uint8_t *buffer, size_t size) {
    eval_vtbl(self, write, buffer, size);
}

inline void gpio_read(Gpio *self, uint8_t *buffer, size_t size) {
    eval_vtbl(self, read, buffer, size);
}

inline void gpio_end(Gpio *self) {
    eval_vtbl(self, end);
}
// end define gpio

// define i2c
static void _i2c_begin(I2c *driver) {
    DEFAULT_METHOD();
}

static void _i2c_init(I2c *driver, const I2cInfo *info) {
    DEFAULT_METHOD();
}

static void _i2c_write(I2c *driver, const uint8_t *buffer, size_t size) {
    DEFAULT_METHOD();
}

static void _i2c_read(I2c *driver, uint8_t *buffer, size_t size) {
    DEFAULT_METHOD();
}

static void _i2c_end(I2c *driver) {
    DEFAULT_METHOD();
}

static I2cVTbl _i2c_vtbl = {
        .begin = _i2c_begin,
        .init = _i2c_init,
        .write = _i2c_write,
        .read = _i2c_read,
        .end = _i2c_end,
};

inline I2c *new_i2c() {
    I2c *i2c = (I2c *) malloc(sizeof(I2c));
    assert(i2c);
    link(i2c, "I2C", del_i2c);
    i2c->vtbl = &_i2c_vtbl;
    return i2c;
}

inline void del_i2c(void *self) {
    if (self) {
        I2c *_self = (I2c *) self;
        object_delete(_self->obj);
    }
    free(self);
    self = NULL;
}

inline void i2c_begin(I2c *self) {
    eval_vtbl(self, begin);
}

inline void i2c_init(I2c *self, const I2cInfo *info) {
    eval_vtbl(self, init, info);
}

inline void i2c_write(I2c *self, const uint8_t *data, size_t size) {
    eval_vtbl(self, write, data, size);
}

inline void i2c_read(I2c *self, uint8_t *buffer, size_t size) {
    eval_vtbl(self, read, buffer, size);
}

inline void i2c_end(I2c *self) {
    eval_vtbl(self, end);
}
// end define i2c

// define spi
static void _spi_begin(Spi *driver) {
    DEFAULT_METHOD();
}

static void _spi_init(Spi *driver, const SpiInfo *info) {
    DEFAULT_METHOD();
}

static void _spi_write(Spi *driver, const uint8_t *buffer, size_t size) {
    DEFAULT_METHOD();
}

static void _spi_read(Spi *driver, uint8_t *buffer, size_t size) {
    DEFAULT_METHOD();
}

static void _spi_end(Spi *driver) {
    DEFAULT_METHOD();
}

static SpiVTbl _spi_vtbl = {
        .begin = _spi_begin,
        .init = _spi_init,
        .write = _spi_write,
        .read = _spi_read,
        .end = _spi_end,
};

inline Spi *new_spi() {
    Spi *spi = (Spi *) malloc(sizeof(Spi));
    assert(spi);
    link(spi, "SPI", del_spi);
    spi->vtbl = &_spi_vtbl;
    return spi;
}

inline void del_spi(void *self) {
    if (self) {
        Spi *_self = (Spi *) self;
        object_delete(_self->obj);
    }
    free(self);
    self = NULL;
}

inline void spi_begin(Spi *self) {
    eval_vtbl(self, begin);
}

inline void spi_init(Spi *self, const SpiInfo *info) {
    eval_vtbl(self, init, info);
}

inline void spi_write(Spi *self, const uint8_t *data, size_t size) {
    eval_vtbl(self, write, data, size);
}

inline void spi_read(Spi *self, uint8_t *buffer, size_t size) {
    eval_vtbl(self, read, buffer, size);
}

inline void spi_end(Spi *self) {
    eval_vtbl(self, end);
}
// end define spi