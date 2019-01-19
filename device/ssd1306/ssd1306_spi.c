//
// Created by mumumusuc on 19-1-19.
//

#include "ssd1306.h"
#include "ssd1306_private.h"

static void write_data(Ssd1306Display *self, uint8_t data) {
    LOG("spi : write_data");
}

static void write_cmd(Ssd1306Display *self, uint8_t cmd) {
    LOG("spi : write_cmd");
}

Ssd1306Display *new_Ssd1306Display_SPI() {
    Ssd1306Display *display = new_Ssd1306Display();
    display->write_data = write_data;
    display->write_cmd = write_cmd;
    return display;
}