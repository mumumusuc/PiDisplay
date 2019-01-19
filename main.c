//
// Created by mumumusuc on 19-1-19.
//

#include <stdio.h>
#include "common.h"
#include "display.h"
#include "device/ssd1306/ssd1306.h"

int main(int argc, char *argv[]) {

    DisplayInfo info = {};
    BaseDisplay *p = new_Ssd1306Display_I2C();
    p->init(p);
    p->reset(p);
    p->get_info(p, &info);
    printf(
            "w = %d, h = %d, format = %d, vendor = %s.\n",
            info.width,
            info.height,
            info.pixel_format,
            info.vendor
    );
    delete_Ssd1306Display(p);

    return 0;
}