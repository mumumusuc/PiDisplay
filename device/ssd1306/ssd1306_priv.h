//
// Created by mumumusuc on 19-1-19.
//

#ifndef PI_DISPLAY_SSD1306_PRIVATE_H
#define PI_DISPLAY_SSD1306_PRIVATE_H

#include "ssd1306.h"
#include "display_priv.h"

Ssd1306 *new_Ssd1306(GPIO *, uint8_t rst);

void del_Ssd1306(Ssd1306 *);

void init_Ssd1306(Ssd1306 *, GPIO *, SsdOps *ops, uint8_t rst);

static void begin(Display *);

static void reset(Display *);

static void turn_on(Display *);

static void clear(Display *);

static void update(Display *, void *);

static void turn_off(Display *);

static void end(Display *);

#endif //PI_DISPLAY_SSD1306_PRIVATE_H
