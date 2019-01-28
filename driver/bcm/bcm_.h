//
// Created by mumumusuc on 19-1-27.
//

#ifndef PI_DISPLAY_BCM_H
#define PI_DISPLAY_BCM_H

#include "driver_.h"

#ifdef __cplusplus
extern "C" {
#endif
// gpio
typedef struct _BcmGpio {
    Object *obj;
} BcmGpio;

BcmGpio *new_bcm_gpio() API_WEAK;

void del_bcm_gpio(void *) API_WEAK;
// end gpio

// i2c
typedef struct _BcmI2c {
    Object *obj;
} BcmI2c;

BcmI2c *new_bcm_i2c() API_WEAK;

void del_bcm_i2c(void *) API_WEAK;
//end i2c

// spi
typedef struct _BcmSpi {
    Object *obj;
} BcmSpi;

BcmSpi *new_bcm_spi() API_WEAK;

void del_bcm_spi(void *) API_WEAK;
//end spi

#ifdef __cplusplus
}
#endif

#endif //PI_DISPLAY_BCM_H
