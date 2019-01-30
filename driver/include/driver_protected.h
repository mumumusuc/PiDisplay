//
// Created by mumumusuc on 19-1-27.
//

#ifndef PI_DISPLAY_DRIVER_PROTECTED_H
#define PI_DISPLAY_DRIVER_PROTECTED_H

#include "driver.h"

#ifdef __cplusplus
extern "C" {
#endif

// define gpio protected
typedef void(*fpGpioBegin)(Gpio *);

typedef void(*fpGpioInit)(Gpio *, const GpioInfo *);

typedef void(*fpGpioWrite)(Gpio *, const uint8_t *, size_t);

typedef void(*fpGpioRead)(Gpio *, uint8_t *, size_t);

typedef void(*fpGpioEnd)(Gpio *);

struct _Gpio_VTbl {
    fpGpioBegin begin;
    fpGpioInit init;
    fpGpioWrite write;
    fpGpioRead read;
    fpGpioEnd end;
};

void del_gpio(void *);
// end define gpio protected

// define i2c protected
typedef void(*fpI2cBegin)(I2c *);

typedef void(*fpI2cInit)(I2c *, const I2cInfo *);

typedef void(*fpI2cWrite)(I2c *, const uint8_t *, size_t);

typedef void(*fpI2cRead)(I2c *, uint8_t *, size_t);

typedef void(*fpI2cEnd)(I2c *);

struct _I2c_VTbl {
    fpI2cBegin begin;
    fpI2cInit init;
    fpI2cWrite write;
    fpI2cRead read;
    fpI2cEnd end;
};

void del_i2c(void *);
// end define i2c protected

// define spi protected
typedef void(*fpSpiBegin)(Spi *, Gpio *);

typedef void(*fpSpiInit)(Spi *, const SpiInfo *);

typedef void(*fpSpiWrite)(Spi *, const uint8_t *, size_t);

typedef void(*fpSpiRead)(Spi *, uint8_t *, size_t);

typedef void(*fpSpiEnd)(Spi *);

struct _Spi_VTbl {
    fpSpiBegin begin;
    fpSpiInit init;
    fpSpiWrite write;
    fpSpiRead read;
    fpSpiEnd end;
};

void del_spi(void *);
// end define spi protected

#ifdef __cplusplus
}
#endif

#endif //PI_DISPLAY_DRIVER_PROTECTED_H
