//
// Created by mumumusuc on 19-1-23.
//

#ifndef PI_DISPLAY_FACTORY_H
#define PI_DISPLAY_FACTORY_H

#include "display.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEVICE_NUM  4

const char *device_type[DEVICE_NUM] = {
        "/ssd1306/bcm/i2c",
        "/ssd1306/bcm/spi",
        "/ssd1306/default/i2c",
        "/ssd1306/default/spi",
};

// define device factory
Display *create_display(const char *);

// end define device factory
#ifdef __cplusplus
}
#endif
#endif //PI_DISPLAY_FACTORY_H
