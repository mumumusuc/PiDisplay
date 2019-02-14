
#ifndef PI_DISPLAY_SSD1306_H
#define PI_DISPLAY_SSD1306_H

#include <linux/types.h>
#include <linux/mutex.h>
#include "ssd1306_def.h"

#define DISPLAY_WIDTH   SCREEN_COLUMNS
#define DISPLAY_HEIGHT  SCREEN_ROWS
#define DISPLAY_SIZE    (DISPLAY_WIDTH*DISPLAY_HEIGHT/8)

struct display;

struct interface {
    u8 gpio_reset;

    ssize_t (*write_cmd)(struct display *, u8);

    ssize_t (*write_data)(struct display *, u8);

    ssize_t (*write_data_buffer)(struct display *, const u8 *, size_t);
};

struct display {
    void *par;
    u8 gpio_reset;
    u8 *vmem;
    size_t vmem_size;
    struct mutex mem_mutex;
    struct interface interface;
};

// define init & deinit
int display_init(struct display *, struct interface *);

void display_deinit(struct display *);

int display_driver_probe(struct device *, struct display *, struct interface *);

void dislay_driver_remove(struct display *);

// define display methods
void display_reset(struct display *);

void display_turn_on(struct display *);

void display_clear(struct display *);

void display_update(struct display *, size_t);

void display_turn_off(struct display *);

// define driver(spi & i2c) methods
int display_driver_spi_init(void);

void display_driver_spi_exit(void);

int display_driver_i2c_init(void);

void display_driver_i2c_exit(void);

#endif //PI_DISPLAY_SSD1306_H
