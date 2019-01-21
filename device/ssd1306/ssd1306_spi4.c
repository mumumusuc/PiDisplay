//
// Created by mumumusuc on 19-1-19.
//

#include <assert.h>
#include "common.h"
#include "ssd1306_priv.h"

static void write_data(Ssd1306Display *self, uint8_t data) {
    LOG("spi : write_data");
    Ssd1306Display_SPI4 *_self = container_of(self, Ssd1306Display_SPI4, spi4);
    _self->spi4->write(NULL, NULL);
}

static void write_cmd(Ssd1306Display *self, uint8_t cmd) {
    LOG("spi : write_cmd");
    Ssd1306Display_SPI4 *_self = container_of(self, Ssd1306Display_SPI4, spi4);
    _self->spi4->write(NULL, NULL);
}

Ssd1306Display *new_Ssd1306Display_SPI4(const GPIO *gpio, const SPI *spi4) {
    assert(spi4);
    Ssd1306Display_SPI4 *display = (Ssd1306Display_SPI4 *) malloc(sizeof(Ssd1306Display_SPI4));
    assert(display);
    display->base = new_Ssd1306Display(gpio);
    display->base->write_data = write_data;
    display->base->write_cmd = write_cmd;
    display->spi4 = (SPI *) malloc(sizeof(SPI));
    display->spi4->init = spi4->init;
    display->spi4->uninit = spi4->uninit;
    display->spi4->read = spi4->read;
    display->spi4->write = spi4->write;
    return display;
}