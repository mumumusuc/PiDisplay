//
// Created by mumumusuc on 19-1-19.
//

#ifndef PI_DISPLAY_DRIVER_H
#define PI_DISPLAY_DRIVER_H

#include <stdint.h>

typedef void(*fpDriverInit)(void *);

typedef void(*fpDriverUninit)();

typedef int(*fpDriverWrite)(uint8_t *, size_t);

typedef int(*fpDriverRead)(uint8_t *, size_t);

// define general driver
typedef struct _Driver {
    fpDriverInit init;
    fpDriverUninit uninit;
    fpDriverWrite write;
    fpDriverRead read;
} Driver;

void init_Driver(Driver *restrict, Driver *restrict);
// end define general driver

// define gpio
typedef struct _GPIOInfo {
    uint8_t pin;
    uint8_t mode;
} GPIOInfo;

typedef struct _GPIO {
    Driver base;
} GPIO;

void init_GPIO(GPIO *, GPIO *);

// end define gpio

// define i2c
typedef struct _I2CInfo {
    uint8_t address;
    uint32_t baudrate;
} I2CInfo;

typedef struct _I2C {
    Driver base;
} I2C;
// end define i2c

// define spi
typedef struct _SPIInfo {
    uint8_t cs;
    uint32_t speed;
} SPIInfo;

typedef struct _SPI {
    Driver base;
} SPI;
// end define spi

#endif //PI_DISPLAY_DRIVER_H
