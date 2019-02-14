//
// Created by mumumusuc on 19-2-9.
//

#ifndef PI_DISPLAY_SSD1306_DEV_H
#define PI_DISPLAY_SSD1306_DEV_H

#include <linux/fb.h>
#include <linux/mutex.h>
#include "ssd1036_chr.h"
#include "ssd1306_def.h"

#define GPIO_LO 0
#define GPIO_HI 1

typedef struct interface {
    void *par;
    u8 gpio_reset;

    ssize_t (*write_cmd)(struct interface *, u8);

    ssize_t (*write_data)(struct interface *, u8);

    ssize_t (*write_data_buffer)(struct interface *, const u8 *, size_t);
} interface_t;

typedef struct display {
    u8 *vmem;
    size_t vmem_size;
    struct mutex mem_mutex;
    struct interface *interface;
} display_t;

static display_info_t ssd1306_info = {
        .width = SCREEN_COLUMNS,
        .height = SCREEN_ROWS,
        .format = 1,
        .vendor = "ssd1306_128_64",
        .size = 1024,
};

int display_init(display_t *);

void display_reset(display_t *);

void display_turn_on(display_t *);

void display_clear(display_t *);

void display_update(display_t *, const u8 *, size_t);

void display_turn_off(display_t *);

void display_deinit(display_t *);

void *display_fb_get_par(struct device *);

int display_fb_probe(struct device *device, struct interface *);

void display_fb_remove(struct device *device);

#endif //PI_DISPLAY_SSD1306_DEV_H
