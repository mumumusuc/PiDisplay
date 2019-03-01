//
// Created by mumumusuc on 19-1-25.
//

#ifndef PI_DISPLAY_SSD1306_PROTECTED_H
#define PI_DISPLAY_SSD1306_PROTECTED_H

#include "ili9341_dev.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void(*fpSSDBegin)(SSD1306 *);

typedef void(*fpSSDEnd)(SSD1306 *);

typedef void(*fpSSDWriteData)(SSD1306 *, uint8_t);

typedef void(*fpSSDWriteCmd)(SSD1306 *, uint8_t);

struct _SSDVTbl {
    fpSSDBegin begin_com;
    fpSSDEnd end_com;
    fpSSDWriteData write_data;
    fpSSDWriteCmd write_cmd;
};

void del_ssd1306(void *);

void del_ssd1306_i2c(void *);

void del_ssd1306_spi4(void *);

#ifdef __cplusplus
}
#endif

#endif //PI_DISPLAY_SSD1306_PROTECTED_H
