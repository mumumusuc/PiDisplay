//
// Created by mumumusuc on 19-2-9.
//

#ifndef PI_DISPLAY_SSD1306_DEV_H
#define PI_DISPLAY_SSD1306_DEV_H

#include "ssd1036.h"
#include "ssd1306_def.h"

#define GPIO_LO 0
#define GPIO_HI 1

typedef struct display {
    u8 *screen;
    u8 gpio_reset;

    ssize_t (*write_cmd)(struct display *, u8);

    ssize_t (*write_data)(struct display *, u8);

    ssize_t (*write_data_buffer)(struct display *, const u8 *, size_t);
} display_t;

int display_init(display_t *);

void display_reset(display_t *);

void display_turn_on(display_t *);

void display_clear(display_t *);

void display_update(display_t *, size_t);

void display_turn_off(display_t *);

void display_deinit(display_t *);

#endif //PI_DISPLAY_SSD1306_DEV_H
