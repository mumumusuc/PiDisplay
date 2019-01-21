//
// Created by mumumusuc on 19-1-19.
//

#include <stdio.h>
#include <zconf.h>
#include <string.h>
#include "common.h"
#include "device/include/display.h"

int main(int argc, char *argv[]) {
    printf("Hello world.\n");
    printf("Sizeof(BaseDisplay) = %d.\n", sizeof(BaseDisplay));

    BaseDisplay *display = new_Display();
    display->ops->init(display);
    delete_Display(display);

/*
    BcmGPIO *gpio = new_BcmGPIO();
    GPIOInfo info = {
            .pin=21,
            .mode=1,
    };
    gpio->init((void*)&info);
    int i = 5;
    while (i-- > 0) {
        gpio->write(&(info.pin), 1);
        usleep(500 * 1000);
        gpio->write(&(info.pin), 0);
        usleep(500 * 1000);
    }
    gpio->uninit();
    delete_BcmGPIO(gpio);
    */
/*
    DisplayInfo info = {};
    GPIO* gpio = new_Driver();
    I2C* i2c = new_Driver();
    BaseDisplay *p = (BaseDisplay *) new_Ssd1306Display_I2C(gpio, i2c);
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
    delete_Ssd1306Display((Ssd1306Display *) p);
*/
    return 0;
}