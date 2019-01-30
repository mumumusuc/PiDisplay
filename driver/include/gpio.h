//
// Created by mumumusuc on 19-1-25.
//

#ifndef PI_DISPLAY_GPIO_H
#define PI_DISPLAY_GPIO_H

#include <stdint.h>
#include "object.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GPIO_LOW            0x00
#define GPIO_HIGH           0x01

#define GPIO_MODE_INPUT     0x00
#define GPIO_MODE_OUTPUT    0x01
#define GPIO_MODE_ALT0      0x04

typedef struct _Gpio Gpio;
typedef struct _Gpio_VTbl GpioVTbl;

typedef struct _GpioInfo {
    uint8_t pin;
    uint8_t mode;
} GpioInfo;

struct _Gpio {
    GpioVTbl *vtbl;
    Object *obj;
};

Gpio *new_gpio(void);

void gpio_begin(Gpio *);

void gpio_init(Gpio *, const GpioInfo *);

void gpio_write(Gpio *, const uint8_t *, size_t);

void gpio_read(Gpio *, uint8_t *, size_t);

void gpio_end(Gpio *);

#ifdef __cplusplus
}
#endif

#endif //PI_DISPLAY_GPIO_H
