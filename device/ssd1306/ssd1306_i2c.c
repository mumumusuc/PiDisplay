//
// Created by mumumusuc on 19-1-19.
//


#include <assert.h>
#include "common.h"
#include "ssd1306_def.h"
#include "ssd1306_priv.h"

static void init_com(Ssd1306Display *self) {
    Ssd1306Display_I2C *_self = container_of(self, Ssd1306Display_I2C, base);
    I2CInfo info = {
            .address = SSD1306_I2C_ADDR,
            .baudrate = SSD1306_I2C_RATE,
    };
    _self->i2c.init(&info);
}

static void uninit_com(Ssd1306Display *self) {
    Ssd1306Display_I2C *_self = container_of(self, Ssd1306Display_I2C, base);
    _self->i2c.uninit();
}

static void write_data(Ssd1306Display *self, uint8_t data) {
   // LOG("i2c : write_data");
    Ssd1306Display_I2C *_self = container_of(self, Ssd1306Display_I2C, base);

    uint8_t temp[2] = {0};
    temp[0] = 0x40;
    temp[1] = data;

    _self->i2c.write(temp, 2);
}

static void write_cmd(Ssd1306Display *self, uint8_t cmd) {
  //  LOG("i2c : write_cmd");
    Ssd1306Display_I2C *_self = container_of(self, Ssd1306Display_I2C, base);

    uint8_t data[2] = {0};
    data[0] = 0x00;
    data[1] = cmd;

    _self->i2c.write(data, 2);
}

Ssd1306Display *new_Ssd1306Display_I2C(GPIO *gpio, I2C *i2c) {
    assert(gpio || i2c);
    Ssd1306Display_I2C *display = (Ssd1306Display_I2C *) malloc(sizeof(Ssd1306Display_I2C));
    assert(display);
    init_Ssd1306Display(display, gpio);
    display->base.init_com = init_com;
    display->base.uninit_com = uninit_com;
    display->base.write_data = write_data;
    display->base.write_cmd = write_cmd;
    init_Driver(&(display->i2c), &(i2c->base));
    return display;
}