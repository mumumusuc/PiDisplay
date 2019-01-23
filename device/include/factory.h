//
// Created by mumumusuc on 19-1-23.
//

#ifndef PI_DISPLAY_FACTORY_H
#define PI_DISPLAY_FACTORY_H

#include "display_priv.h"

typedef enum {
    DISPLAY_NONE = 0,
    DISPLAY_SSD1306_BCM_I2C,
    DISPLAY_SSD1306_BCM_SPI4,
    DISPLAY_SSD1306_DEF_I2C,
    DISPLAY_SSD1306_DEF_SPI4,
} DisplayType;

// define device factory
Display *create_display(DisplayType);

void destroy_display(Display *);
// end define device factory

#endif //PI_DISPLAY_FACTORY_H
