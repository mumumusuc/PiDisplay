//
// Created by mumumusuc on 19-1-19.
//

#ifndef PI_DISPLAY_SSD1306_H
#define PI_DISPLAY_SSD1306_H

#include "display.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void(*fpWriteData)(void *, uint8_t);

typedef void(*fpWriteCmd)(void *, uint8_t);

// define ssd1306
typedef struct _Ssd1306Display {
    const BaseDisplay *parent;
    fpGetInfo get_info;
    fpInit init;
    fpReset reset;
    fpTurnOn turn_on;
    fpTurnOff turn_off;
    fpUpdate update;
    // virtual methods
    fpWriteData write_data;
    fpWriteCmd write_cmd;
} Ssd1306Display;

Ssd1306Display *new_Ssd1306Display_I2C();

Ssd1306Display *new_Ssd1306Display_SPI();

void delete_Ssd1306Display(Ssd1306Display *);
// end define ssd1306

#ifdef __cplusplus
}
#endif
#endif //PI_DISPLAY_SSD1306_H
