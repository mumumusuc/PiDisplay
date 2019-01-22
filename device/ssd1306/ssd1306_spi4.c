//
// Created by mumumusuc on 19-1-19.
//

#include <assert.h>
#include "common.h"
#include "ssd1306_def.h"
#include "ssd1306_priv.h"

static void init_com(Ssd1306 *self) {
    Ssd1306_SPI4 *_self = container_of(self, Ssd1306_I2C, base);
    SPIInfo info = {
            .cs = 0,
            .speed = 8000000,
    };
    SPI spi = _self->spi4;
    spi.ops.init(&spi, &info);
    GPIOInfo gpio_info = {
            .pin = HW_DC_PIN,
            .mode = GPIO_MODE_OUTPUT,
    };
    self->gpio.ops.init(&(self->gpio), &gpio_info);
}

static void uninit_com(Ssd1306 *self) {
    Ssd1306_SPI4 *_self = container_of(self, Ssd1306_I2C, base);
    SPI spi = _self->spi4;
    spi.ops.uninit(&spi);
}

static void write_data(Ssd1306 *self, uint8_t data) {
    //LOG("spi : write_data");
    Ssd1306_SPI4 *_self = container_of(self, Ssd1306_SPI4, base);
    SPI spi = _self->spi4;
    uint8_t pin = HW_DC_PIN;
    self->gpio.ops.write(&(self->gpio), &pin, GPIO_HIGH);
    spi.ops.write(&spi, &data, 1);
}

static void write_cmd(Ssd1306 *self, uint8_t cmd) {
    //LOG("spi : write_cmd");
    Ssd1306_SPI4 *_self = container_of(self, Ssd1306_SPI4, base);
    SPI spi = _self->spi4;
    uint8_t pin = HW_DC_PIN;
    self->gpio.ops.write(&(self->gpio), &pin, GPIO_LOW);
    spi.ops.write(&spi, &cmd, 1);
}

Ssd1306_SPI4 *new_Ssd1306_SPI4(GPIO *gpio, SPI *spi4) {
    assert(gpio || spi4);
    Ssd1306_SPI4 *display = (Ssd1306_SPI4 *) malloc(sizeof(Ssd1306_SPI4));
    assert(display);
    SsdOps ssd_ops = {
            .init_com = init_com,
            .uninit_com = uninit_com,
            .write_cmd = write_cmd,
            .write_data = write_data,
    };
    init_Ssd1306(&(display->base), gpio, &ssd_ops, HW_RST_PIN_2);
    init_SPI(&(display->spi4), &(spi4->ops));
    return display;
}

void del_Ssd1306_SPI4(Ssd1306_SPI4 *self) {
    free(self);
    self = NULL;
}