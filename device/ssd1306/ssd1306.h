//
// Created by mumumusuc on 19-1-19.
//

#ifndef PI_DISPLAY_SSD1306_H
#define PI_DISPLAY_SSD1306_H

#include "display.h"
#include "driver.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void(*fpWriteData)(struct _Ssd1306Display *, uint8_t);

typedef void(*fpWriteCmd)(struct _Ssd1306Display *, uint8_t);

// define ssd1306
typedef struct _Ssd1306Display {
    BaseDisplay *base;
    GPIO *gpio;
    fpWriteCmd write_cmd;
    fpWriteData write_data;
} Ssd1306Display;

// define ssd1306_i2c
typedef struct _Ssd1306Display_I2C {
    Ssd1306Display *base;
    I2C *i2c;
} Ssd1306Display_I2C;

// define ssd1306_spi4
typedef struct _Ssd1306Display_SPI4 {
    Ssd1306Display *base;
    SPI *spi4;
} Ssd1306Display_SPI4;

Ssd1306Display *new_Ssd1306Display_I2C(GPIO *, I2C *);

Ssd1306Display *new_Ssd1306Display_SPI(GPIO *, SPI *);

void delete_Ssd1306Display(Ssd1306Display *);
// end define ssd1306

#ifdef __cplusplus
}
#endif
#endif //PI_DISPLAY_SSD1306_H
