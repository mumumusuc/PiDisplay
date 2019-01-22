//
// Created by mumumusuc on 19-1-19.
//


#include <assert.h>
#include "common.h"
#include "ssd1306_def.h"
#include "ssd1306_priv.h"

static void init_com(Ssd1306 *self) {
    LOG("i2c : init_com");
    Ssd1306_I2C *_self = container_of(self, Ssd1306_I2C, base);
    I2CInfo info = {
            .address = SSD1306_I2C_ADDR,
            .baudrate = SSD1306_I2C_RATE,
    };
    I2C i2c = _self->i2c;
    i2c.ops.init(&i2c, &info);
    LOG("i2c : init_com end");
}

static void uninit_com(Ssd1306 *self) {
    LOG("i2c : uninit_com");
    Ssd1306_I2C *_self = container_of(self, Ssd1306_I2C, base);
    I2C i2c = _self->i2c;
    i2c.ops.uninit(&i2c);
}

static void write_data(Ssd1306 *self, uint8_t data) {
    //LOG("i2c : write_data");
    Ssd1306_I2C *_self = container_of(self, Ssd1306_I2C, base);
    I2C i2c = _self->i2c;
    uint8_t temp[2] = {0};
    temp[0] = 0x40;
    temp[1] = data;
    i2c.ops.write(&i2c, temp, 2);
}

static void write_cmd(Ssd1306 *self, uint8_t cmd) {
    //LOG("i2c : write_cmd");
    Ssd1306_I2C *_self = container_of(self, Ssd1306_I2C, base);
    I2C i2c = _self->i2c;
    uint8_t data[2] = {0};
    data[0] = 0x00;
    data[1] = cmd;
    i2c.ops.write(&i2c, data, 2);
}

Ssd1306_I2C *new_Ssd1306_I2C(GPIO *gpio, I2C *i2c) {
    assert(gpio || i2c);
    Ssd1306_I2C *display = (Ssd1306_I2C *) malloc(sizeof(Ssd1306_I2C));
    assert(display);
    SsdOps ssd_ops = {
            .init_com = init_com,
            .uninit_com = uninit_com,
            .write_cmd = write_cmd,
            .write_data = write_data,
    };
    init_Ssd1306(&(display->base), gpio, &ssd_ops, HW_RST_PIN_1);
    init_I2C(&(display->i2c), &(i2c->ops));
    return display;
}

void del_Ssd1306_I2C(Ssd1306_I2C *self) {
    if (self) {
        //   del_Ssd1306(&(self->base));
    }
    free(self);
    self = NULL;
}