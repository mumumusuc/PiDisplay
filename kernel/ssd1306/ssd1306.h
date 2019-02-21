
#ifndef PI_DISPLAY_SSD1306_H
#define PI_DISPLAY_SSD1306_H

#include <linux/types.h>
#include <linux/mutex.h>
#include "ssd1306_def.h"

#define DISPLAY_MODULE  "SSD1306_128_64"
#define DISPLAY_WIDTH   SCREEN_COLUMNS
#define DISPLAY_HEIGHT  SCREEN_ROWS
#define DISPLAY_SIZE    (DISPLAY_WIDTH*DISPLAY_HEIGHT/8)
#define DISPLAY_LINE    DISPLAY_WIDTH

#define DEBUG

#ifdef DEBUG
//#define DEBUG_TIME
#ifdef DEBUG_TIME
#include <linux/time.h>
#endif
#define debug(format, ...)  printk(KERN_DEBUG "[%s] "format"\n",__func__,##__VA_ARGS__)
#else
#define debug(format, ...)  {}
#endif

struct roi {
    int dirty;      /* update display only when dirty is true */
    int row_offset;
    int col_start;
    int row_start;
    int col_end;
    int row_end;
    struct spinlock locker;
};

struct surface {
    u8 bpp;             /* bit per pixel */
    u8 *data;           /* screen buffer */
    size_t width;
    size_t height;
    size_t padding;     /* padding pixels */
    size_t line_length; /* byte per line  */
};

struct display {
    unsigned id;
    u8 gpio_reset;
    struct mutex mem_mutex;
    struct roi *roi;            /* roi anchor, may be NULL */
    struct surface *surface;    /* surface anchor, may be NULL */
    struct interface *interface;
#ifdef  DEBUG_TIME
    struct timeval time;
#endif
};

struct interface {
    ssize_t (*write_cmd)(struct display *, u8);

    ssize_t (*write_data)(struct display *, u8);

    ssize_t (*write_data_buffer)(struct display *, const u8 *, size_t);
};

// define init & deinit
int display_init(struct display *, unsigned id);

void display_deinit(struct display *);

int display_driver_probe(struct device *, struct display *);

void dislay_driver_remove(struct display *);

// define display methods
int display_set_roi(struct display *, int, int, size_t, size_t);

void display_reset(struct display *);

void display_turn_on(struct display *);

void display_clear(struct display *);

void display_update(struct display *, struct surface *);

void display_turn_off(struct display *);

// define driver(spi & i2c) methods
int display_driver_spi_init(void);

void display_driver_spi_exit(void);

int display_driver_i2c_init(void);

void display_driver_i2c_exit(void);

#endif //PI_DISPLAY_SSD1306_H
