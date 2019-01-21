//
// Created by mumumusuc on 19-1-19.
//

#ifndef PI_DISPLAY_DRIVER_H
#define PI_DISPLAY_DRIVER_H

#include <stdint.h>

typedef void(*fpDriverInit)(void *);

typedef void(*fpUninit)();

typedef int(*fpWrite)(uint8_t *, size_t);

typedef int(*fpRead)(uint8_t *, size_t);

typedef struct _Driver {
    fpDriverInit init;
    fpUninit uninit;
    fpWrite write;
    fpRead read;
} Driver;

typedef struct _GPIOInfo {
    uint8_t pin;
    uint8_t mode;
} GPIOInfo;

typedef struct _GPIO {
    fpDriverInit init;
    fpUninit uninit;
    fpWrite write;
    fpRead read;
} GPIO;

typedef struct _I2CInfo {
    uint8_t address;
    uint32_t baudrate;
};

typedef struct _I2C {
    fpDriverInit init;
    fpUninit uninit;
    fpWrite write;
    fpRead read;
} I2C;

typedef struct _SPIInfo {
    uint8_t cs;
    uint32_t speed;
};

typedef struct _SPI {
    fpDriverInit init;
    fpUninit uninit;
    fpWrite write;
    fpRead read;
} SPI;

Driver *new_Driver();

void delete_Driver(Driver *);

#endif //PI_DISPLAY_DRIVER_H
