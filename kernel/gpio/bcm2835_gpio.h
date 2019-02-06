//
// Created by mumumusuc on 19-2-1.
//

#ifndef PI_DISPLAY_BCM2835_GPIO_H
#define PI_DISPLAY_BCM2835_GPIO_H

#define PI_ZERO

#define GPIO_HIGH   (0x01)
#define GPIO_LOW    (0x00)

int gpio_init(void);

int gpio_deinit(void);

void gpio_set_mode(unsigned char pin, unsigned char mode);

unsigned char gpio_get_mode(unsigned char pin);

void gpio_set_value(unsigned char pin, unsigned char level);

unsigned char gpio_get_value(unsigned char pin);

#endif //PI_DISPLAY_BCM2835_GPIO_H
