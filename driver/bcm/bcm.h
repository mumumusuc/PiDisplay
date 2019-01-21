//
// Created by mumumusuc on 19-1-19.
//

#ifndef PI_DISPLAY_BCM_H
#define PI_DISPLAY_BCM_H

#include "driver.h"

// gpio
typedef struct _BcmGPIO {
    GPIO base;
} BcmGPIO;

BcmGPIO *new_BcmGPIO();

void delete_BcmGPIO(BcmGPIO *);
// end gpio

// i2c
typedef struct _BcmI2C {
    I2C base;
} BcmI2C;

BcmI2C *new_BcmI2C();

void delete_BcmI2C(BcmI2C *);
//end i2c

// spi
typedef struct _BcmSPI {
    SPI base;
} BcmSPI;

BcmSPI *new_BcmSPI();

void delete_BcmSPI(BcmSPI *);
//end spi

#endif //PI_DISPLAY_BCM_H
