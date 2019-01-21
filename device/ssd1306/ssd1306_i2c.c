//
// Created by mumumusuc on 19-1-19.
//


#include <assert.h>
#include "common.h"
#include "ssd1306_priv.h"

static void write_data(Ssd1306Display *self, uint8_t data) {
    LOG("i2c : write_data");
    Ssd1306Display_I2C *_self = container_of(self, Ssd1306Display_I2C, i2c);
    _self->i2c->write(NULL, NULL);
}

static void write_cmd(Ssd1306Display *self, uint8_t cmd) {
    LOG("i2c : write_cmd");
    Ssd1306Display_I2C *_self = container_of(self, Ssd1306Display_I2C, i2c);
    _self->i2c->write(NULL, NULL);
}

Ssd1306Display *new_Ssd1306Display_I2C(GPIO *gpio, I2C *i2c) {
    assert(i2c);
    Ssd1306Display_I2C *display = (Ssd1306Display_I2C *) malloc(sizeof(Ssd1306Display_I2C));
    assert(display);
    display->base = new_Ssd1306Display(gpio);
    display->base->write_data = write_data;
    display->base->write_cmd = write_cmd;
    display->i2c = (I2C *) malloc(sizeof(I2C));
    i2c->init = i2c->init;
    i2c->uninit = i2c->uninit;
    i2c->read = i2c->read;
    i2c->write = i2c->write;
    return display;
}