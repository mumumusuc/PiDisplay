//
// Created by mumumusuc on 19-1-19.
//


#include <assert.h>
#include "common.h"
#include "ssd1306_def.h"
#include "ssd1306_protected.h"

#undef  LOG_TAG
#define LOG_TAG     "SSD1306_I2C"
#define I2C_RST     17

static uint8_t data_tmp[2] = {0x40, 0x00};
static uint8_t cmd_tmp[2] = {0x00, 0x00};

static void _begin_com(SSD1306 *self) {
    LOG("%s", __func__);
    SSD1306_I2C *_self = subclass(self, SSD1306_I2C);
    I2cInfo info = {
            .address = SSD1306_I2C_ADDR,
            .baudrate = SSD1306_I2C_RATE,
    };
    i2c_begin(_self->i2c);
    i2c_init(_self->i2c, &info);
}

static void _end_com(SSD1306 *self) {
    LOG("%s", __func__);
    SSD1306_I2C *_self = subclass(self, SSD1306_I2C);
    i2c_end(_self->i2c);
}

static void _write_data(SSD1306 *self, uint8_t data) {
    //LOG("%s", __func__);
    SSD1306_I2C *_self = subclass(self, SSD1306_I2C);
    data_tmp[1] = data;
    i2c_write(_self->i2c, data_tmp, 2);
}

static void _write_cmd(SSD1306 *self, uint8_t cmd) {
    //LOG("%s", __func__);
    SSD1306_I2C *_self = subclass(self, SSD1306_I2C);
    cmd_tmp[1] = cmd;
    i2c_write(_self->i2c, cmd_tmp, 2);
}

static SSDVTbl _vtbl = {
        .begin_com = _begin_com,
        .end_com = _end_com,
        .write_cmd = _write_cmd,
        .write_data = _write_data,
};

SSD1306_I2C *new_ssd1306_i2c(Gpio *gpio, I2c *i2c) {
    LOG("%s", __func__);
    assert(gpio && i2c);
    SSD1306_I2C *display = (SSD1306_I2C *) malloc(sizeof(SSD1306_I2C));
    assert(display);
    display->i2c = i2c;
    SSD1306 *super = new_ssd1306(gpio, I2C_RST);
    super->vtbl = &_vtbl;
    link2(display, super, "SSD1306_I2C", del_ssd1306_i2c);
    return display;
}

void del_ssd1306_i2c(void *self) {
    LOG("%s", __func__);
    if (self) {
        SSD1306_I2C *_self = (SSD1306_I2C *) self;
        object_delete(_self->obj);
        delete(_self->i2c->obj);
    }
    free(self);
    self = NULL;
}