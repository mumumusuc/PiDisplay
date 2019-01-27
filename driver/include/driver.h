//
// Created by mumumusuc on 19-1-19.
//

#ifndef PI_DISPLAY_DRIVER_H
#define PI_DISPLAY_DRIVER_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct _DriverOps DriverOps;

typedef void(*fpDriverInit)(void *, void *);

typedef void(*fpDriverUninit)(void *);

typedef int(*fpDriverWrite)(void *, uint8_t *, size_t);

typedef int(*fpDriverRead)(void *, uint8_t *, size_t);

// define general driver
struct _DriverOps {
    fpDriverInit init;
    fpDriverUninit uninit;
    fpDriverWrite write;
    fpDriverRead read;
};

// end define general driver

// define gpio
#define GPIO_LOW            0x00
#define GPIO_HIGH           0x01
#define GPIO_MODE_INPUT     0x00
#define GPIO_MODE_OUTPUT    0x01
typedef struct _GPIOInfo {
    uint8_t pin;
    uint8_t mode;
} GPIOInfo;

typedef struct _GPIO {
    DriverOps ops;
} GPIO;

GPIO *new_GPIO();

void del_GPIO(GPIO *);

void init_GPIO(GPIO *, DriverOps *);
// end define gpio

// define i2c
typedef struct _I2CInfo {
    uint8_t address;
    uint32_t baudrate;
} I2CInfo;

typedef struct _I2C {
    DriverOps ops;
} I2C;

I2C *new_I2C();

void del_I2C(I2C *);

void init_I2C(I2C *, DriverOps *);
// end define i2c

// define spi
typedef struct _SPIInfo {
    uint8_t cs;
    uint32_t speed;
} SPIInfo;

typedef struct _SPI {
    DriverOps ops;
} SPI;

SPI *new_SPI();

void del_SPI(SPI *);

void init_SPI(SPI *, DriverOps *);
// end define spi
#ifdef __cplusplus
}
#endif
#endif //PI_DISPLAY_DRIVER_H
