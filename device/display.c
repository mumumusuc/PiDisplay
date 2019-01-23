//
// Created by mumumusuc on 19-1-19.
//

#include <assert.h>
#include <string.h>
#include "common.h"
#include "display_priv.h"

#define LOG_TAG "DISPLAY"

// default methods
static void begin(Display *self) {
#ifdef DEBUG
    ERROR("begin");
#else
    METHOD_NOT_IMPLEMENTED("begin");
#endif
}

static void reset(Display *self) {
#ifdef DEBUG
    ERROR("reset");
#else
    METHOD_NOT_IMPLEMENTED("reset");
#endif
}

static void turn_on(Display *self) {
#ifdef DEBUG
    ERROR("turn_on");
#else
    METHOD_NOT_IMPLEMENTED("turn_on");
#endif
}

static void clear(Display *self) {
#ifdef DEBUG
    ERROR("clear");
#else
    METHOD_NOT_IMPLEMENTED("clear");
#endif
}

static void update(Display *self, void *buffer) {
#ifdef DEBUG
    ERROR("update");
#else
    METHOD_NOT_IMPLEMENTED("update");
#endif
}

static void turn_off(Display *self) {
#ifdef DEBUG
    ERROR("turn_off");
#else
    METHOD_NOT_IMPLEMENTED("turn_off");
#endif
}


static void end(Display *self) {
#ifdef DEBUG
    ERROR("end");
#else
    METHOD_NOT_IMPLEMENTED("end");
#endif
}

// constructor & destructor

void init_Base(Display *display, void *this, fpDestroy destructor) {
    assert(this && destructor);
    display->this = this;
    display->destructor = destructor;
}

void init_Display(Display *display, DisplayOps *ops, DisplayInfo *info) {
    assert(display && ops && info);
    // init struct DisplayInfo
    strcpy(display->info.vendor, info->vendor);
    display->info.width = info->width;
    display->info.height = info->height;
    display->info.pixel_format = info->pixel_format;
    // init struct DisplayOps
    DisplayOps *ptrOps = &(display->ops);
    ptrOps->begin = ops->begin;
    ptrOps->reset = ops->reset;
    ptrOps->turn_on = ops->turn_on;
    ptrOps->clear = ops->clear;
    ptrOps->update = ops->update;
    ptrOps->turn_off = ops->turn_off;
    ptrOps->end = ops->end;
    ptrOps = NULL;
}

Display *new_Display() {
    Display *display = (Display *) malloc(sizeof(Display));
    assert(display);
    DisplayInfo info = {
            .width = 0,
            .height = 0,
            .pixel_format = 0,
            .vendor = "BaseDisplay",
    };
    DisplayOps ops = {
            .begin = begin,
            .reset = reset,
            .turn_on = turn_on,
            .clear = clear,
            .update = update,
            .turn_off = turn_off,
            .end = end,
    };
    init_Display(display, &ops, &info);
    init_Base(display, display, del_Display);
    return display;
}

void del_Display(Display *display) {
    free(display);
    display = NULL;
}

// display factory
#include "factory.h"
#include "ssd1306/ssd1306_priv.h"
#include "bcm/bcm.h"

Display *create_display(DisplayType type) {
    Display *display = NULL;
    switch (type) {
        /* Notice:  This case will create a virtual driver from simulating the full process.
                    Only for Valgrind-mem-check.
         */
        case DISPLAY_NONE: {
            GPIO *gpio = new_GPIO();
            display = &(new_Ssd1306(gpio, -1)->base);
            del_GPIO(gpio);
            break;
        }
        case DISPLAY_SSD1306_BCM_I2C: {
            BcmGPIO *gpio = new_BcmGPIO();
            BcmI2C *i2c = new_BcmI2C();
            display = &(new_Ssd1306_I2C(&(gpio->base), &(i2c->base))->base.base);
            del_BcmGPIO(gpio);
            del_BcmI2C(i2c);
            break;
        }
        case DISPLAY_SSD1306_BCM_SPI4: {
            BcmGPIO *gpio = new_BcmGPIO();
            BcmSPI *spi = new_BcmSPI();
            display = &(new_Ssd1306_SPI4(&(gpio->base), &(spi->base))->base.base);
            del_BcmGPIO(gpio);
            del_BcmSPI(spi);
            break;
        }
        case DISPLAY_SSD1306_DEF_I2C: {
            break;
        }
        case DISPLAY_SSD1306_DEF_SPI4: {
            break;
        }
        default:
            break;
    }
    return display;
}

void destroy_display(Display *display) {
    if (display) {
        fpDestroy destructor = display->destructor;
        void *this = display->this;
        destructor(this);
    }
}