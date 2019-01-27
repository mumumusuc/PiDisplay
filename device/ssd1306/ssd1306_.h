//
// Created by mumumusuc on 19-1-25.
//

#ifndef PI_DISPLAY_SSD1306_H
#define PI_DISPLAY_SSD1306_H

#include "display_.h"
#include "driver_.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _SSD1306 SSD1306;
typedef struct _SSDVTbl SSDVTbl;

// define ssd1306
struct _SSD1306 {
    SSDVTbl *vtbl;
    Gpio *gpio;
    uint8_t pin_reset;
    Object *obj;
};

SSD1306 *new_ssd1306(Gpio *, uint8_t);

void del_ssd1306(void *);
// end define ssd1306

// define ssd1306_i2c
typedef struct _SSD_1306_I2C {
    Object *obj;
    I2c *i2c;
} SSD1306_I2C;

SSD1306_I2C *new_ssd1306_i2c(Gpio *, I2c *);

void del_ssd1306_i2c(void *);
// end define ssd1306_i2c

// define ssd1306_spi4
typedef struct _SSD1306_SPI4 {
    Object *obj;
    Spi *spi;
    uint8_t pin_dc;
} SSD1306_SPI4;

SSD1306_SPI4 *new_ssd1306_spi4(Gpio *, Spi *);

void del_ssd1306_spi4(void *);
// end define ssd1306_spi4

#ifdef __cplusplus
}
#endif

#endif //PI_DISPLAY_SSD1306_H
