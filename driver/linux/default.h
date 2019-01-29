//
// Created by mumumusuc on 19-1-19.
//

#ifndef PI_DISPLAY_DEFAULT_H
#define PI_DISPLAY_DEFAULT_H

#include "driver.h"

#ifdef __cplusplus
extern "C" {
#endif

// gpio
typedef struct _GpioPriv GpioPriv;
typedef struct _DefaultGpio {
    Object *obj;
    GpioPriv *priv;
} DefaultGpio;

DefaultGpio *new_default_gpio();

void del_default_gpio(void *);
// end gpio

// i2c
typedef struct _I2cPriv I2cPriv;
typedef struct _DefaultI2c {
    Object *obj;
    I2cPriv *priv;
} DefaultI2c;

DefaultI2c *new_default_i2c();

void del_default_i2c(void *);
//end i2c

// spi
typedef struct _SpiPriv SpiPriv;
typedef struct _DefaultSpi {
    Object *obj;
    SpiPriv *priv;
} DefaultSpi;

DefaultSpi *new_default_spi();

void del_default_spi(void *);
// end spi

#ifdef __cplusplus
}
#endif
#endif //PI_DISPLAY_DEFAULT_H
