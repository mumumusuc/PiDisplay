
#ifndef PI_DISPLAY_DISPLAY_H
#define PI_DISPLAY_DISPLAY_H

#include <linux/types.h>
#include <linux/mutex.h>

//#define DEBUG

#ifdef DEBUG
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
    unsigned padding;
    struct spinlock locker;
};

struct surface {
    u8 *data;           /* screen buffer */
    u8 depth;           /* bit per pixel */
    size_t width;
    size_t height;
    size_t line_length; /* byte per line  */
};

struct option {
    u8 brightness;
    /* other options */
};

struct display {
    unsigned id;
    struct {
        u8 reset;
        u8 spi_dc;
    } gpio;
    struct roi roi;
    struct mutex dev_locker;
    struct interface *interface;
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

void display_driver_remove(struct display *);

// define display methods
int display_set_option(struct display *, struct option *);

int display_set_roi(struct display *, int, int, size_t, size_t, size_t);

void display_reset(struct display *);

void display_turn_on(struct display *);

void display_clear(struct display *);

void display_update(struct display *, struct surface *, struct roi *);

void display_turn_off(struct display *);

// define driver(spi & i2c) methods
int display_driver_spi_init(void);

void display_driver_spi_exit(void);

int display_driver_i2c_init(void);

void display_driver_i2c_exit(void);

#endif
