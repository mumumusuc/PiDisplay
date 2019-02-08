//
// Created by mumumusuc on 19-1-19.
//

#include <assert.h>
#include "common.h"
#include "ssd1306_def.h"
#include "ssd1306_protected.h"

#undef  LOG_TAG
#define LOG_TAG         "SSD1306_SPI4"
#define SPI_CS_PIN      8
#define SPI_CS_0        0
#define SPI_CS_1        1
#define SPI_DC          22
#define SPI_RST         27
#define SPI_MAX_SPEED   8000000

static void _begin_com(SSD1306 *self) {
    LOG("%s", __func__);
    SSD1306_SPI4 *_self = subclass(self, SSD1306_SPI4);
    // init gpio
    gpio_begin(self->gpio);
    SpiInfo info = {
            .mode = SPI_MODE_0,
            .cs = SPI_CS_0,
            .speed = SPI_MAX_SPEED,
    };
    GpioInfo gpio_info = {
            .pin = SPI_DC,
            .mode = GPIO_MODE_OUTPUT,
    };
    gpio_init(self->gpio, &gpio_info);
    // init spi
    spi_begin(_self->spi, self->gpio);
    spi_init(_self->spi, &info);
    /*
     uint8_t value = 0;
     uint8_t pin = 7;// CE1
     gpio_read(self->gpio, &value, pin);
     LOG("CE1 = %X", value);
     value = 0;
     pin = 8;// CE0
     gpio_read(self->gpio, &value, pin);
     LOG("CE0 = %X", value);
     value = 0;
     pin = 9;// MISO
     gpio_read(self->gpio, &value, pin);
     LOG("MISO = %X", value);
     value = 0;
     pin = 10;// MOSI
     gpio_read(self->gpio, &value, pin);
     LOG("MOSI = %X", value);
     value = 0;
     pin = 11;// CLK
     gpio_read(self->gpio, &value, pin);
     LOG("CLK = %X", value);
     */
}

static void _end_com(SSD1306 *self) {
    LOG("%s", __func__);
    SSD1306_SPI4 *_self = subclass(self, SSD1306_SPI4);
    spi_end(_self->spi);
}

static void _write_data(SSD1306 *self, uint8_t data) {
    //LOG("spi : write_data");
    SSD1306_SPI4 *_self = subclass(self, SSD1306_SPI4);
    gpio_write(self->gpio, &(_self->pin_dc), GPIO_HIGH);
    spi_write(_self->spi, &data, 1);
}

static void _write_cmd(SSD1306 *self, uint8_t cmd) {
    //LOG("spi : write_cmd");
    SSD1306_SPI4 *_self = subclass(self, SSD1306_SPI4);
    gpio_write(self->gpio, &(_self->pin_dc), GPIO_LOW);
    spi_write(_self->spi, &cmd, 1);
}

static SSDVTbl _vtbl = {
        .begin_com = _begin_com,
        .end_com = _end_com,
        .write_cmd = _write_cmd,
        .write_data = _write_data,
};

SSD1306_SPI4 *new_ssd1306_spi4(Gpio *gpio, Spi *spi) {
    LOG("%s", __func__);
    assert(gpio && spi);
    SSD1306_SPI4 *display = (SSD1306_SPI4 *) malloc(sizeof(SSD1306_SPI4));
    assert(display);
    display->spi = spi;
    display->pin_dc = SPI_DC;
    SSD1306 *super = new_ssd1306(gpio, SPI_RST);
    super->vtbl = &_vtbl;
    link2(display, super, "SSD1306_SPI4", del_ssd1306_spi4);
    return display;
}

void del_ssd1306_spi4(void *self) {
    LOG("%s", __func__);
    if (self) {
        SSD1306_SPI4 *_self = (SSD1306_SPI4 *) self;
        object_delete(_self->obj);
        delete(_self->spi->obj);
    }
    free(self);
    self = NULL;
}