//
// Created by mumumusuc on 19-1-19.
//

#ifndef PI_DISPLAY_DRIVER_H
#define PI_DISPLAY_DRIVER_H

#include <stdint.h>

typedef void(*fpInit)(void *);

typedef void(*fpUninit)();

typedef int(*fpWrite)(uint8_t *, size_t);

typedef int(*fpRead)(uint8_t *, size_t);

typedef struct _Driver {
    fpInit init;
    fpUninit uninit;
    fpWrite write;
    fpRead read;
} Driver;

typedef struct _GPIO {

} GPIO;

typedef struct _I2C {

} I2C;

typedef struct _SPI {

} SPI;

#endif //PI_DISPLAY_DRIVER_H
