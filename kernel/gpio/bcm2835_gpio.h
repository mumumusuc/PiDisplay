//
// Created by mumumusuc on 19-2-1.
//

#ifndef PI_DISPLAY_BCM2835_GPIO_H
#define PI_DISPLAY_BCM2835_GPIO_H

#include <asm/types.h>

#define PI_ZERO
#define GPIO_HIGH   (0x01)
#define GPIO_LOW    (0x00)

int gpio_init(void);

int gpio_deinit(void);

void gpio_set_mode(u8 pin, u8 mode);

u8 gpio_get_mode(u8 pin);

void gpio_set_value(u8 pin, u8 level);

u8 gpio_get_value(u8 pin);

#endif //PI_DISPLAY_BCM2835_GPIO_H
