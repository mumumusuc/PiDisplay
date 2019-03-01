
#ifndef PI_DISPLAY_ILI9341_H
#define PI_DISPLAY_ILI9341_H

#include <linux/types.h>

#define DISPLAY_MODULE  "ILI9341_320_240"
#define DISPLAY_WIDTH   320
#define DISPLAY_HEIGHT  240
#define DISPLAY_BBP     24
#define DISPLAY_SIZE    (DISPLAY_WIDTH*DISPLAY_HEIGHT*DISPLAY_BBP/8)
#define DISPLAY_LINE    (DISPLAY_WIDTH*DISPLAY_BBP/8)

struct gpio {
    u8 reset;
    u8 led;
    u8 dc;
};

#endif //PI_DISPLAY_ILI9341_H
