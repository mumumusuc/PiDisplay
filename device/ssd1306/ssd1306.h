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

typedef struct _Ssd1306 Ssd1306;

typedef struct _Ssd1306Ops SsdOps;

typedef void(*fpSsdInitCom)(Ssd1306 *);

typedef void(*fpSsdUninitCom)(Ssd1306 *);

typedef void(*fpSsdWriteData)(Ssd1306 *, uint8_t);

typedef void(*fpSsdWriteCmd)(Ssd1306 *, uint8_t);

// define ssd1306
struct _Ssd1306Ops {
    fpSsdInitCom init_com;
    fpSsdUninitCom uninit_com;
    fpSsdWriteCmd write_cmd;
    fpSsdWriteData write_data;
};

struct _Ssd1306 {
    Display base;
    GPIO gpio;
    SsdOps ops;
    uint8_t pin_reset;
};
// end define ssd1306

// define ssd1306_i2c
typedef struct _Ssd1306_I2C {
    Ssd1306 base;
    I2C i2c;
} Ssd1306_I2C;
// end define ssd1306_i2c

// define ssd1306_spi4
typedef struct _Ssd1306_SPI4 {
    Ssd1306 base;
    SPI spi4;
} Ssd1306_SPI4;
// end define ssd1306_spi4

// define constructor & destructor
Ssd1306_I2C *new_Ssd1306_I2C(GPIO *, I2C *);

void del_Ssd1306_I2C(Ssd1306_I2C *);

Ssd1306_SPI4 *new_Ssd1306_SPI4(GPIO *, SPI *);

void del_Ssd1306_SPI4(Ssd1306_SPI4 *);
// end define constructor & destructor

#ifdef __cplusplus
}
#endif
#endif //PI_DISPLAY_SSD1306_H
