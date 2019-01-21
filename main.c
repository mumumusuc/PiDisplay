//
// Created by mumumusuc on 19-1-19.
//

#include <stdio.h>
#include <zconf.h>
#include <string.h>

#include<stdlib.h>
#include<fcntl.h>   //open()
#include<unistd.h>  //close()
#include<errno.h>   //errno
#include<sys/mman.h> //mmap()


#include "common.h"
#include "device/ssd1306/ssd1306.h"
#include "driver/bcm/bcm.h"

int main(int argc, char *argv[]) {
    printf("Hello world.\n");
    printf("Sizeof(Display) = %d.\n", sizeof(Ssd1306Display_I2C));

    GPIO *gpio = new_BcmGPIO();
    I2C *i2c = new_BcmI2C();
    BaseDisplay *dsp = &(new_Ssd1306Display_I2C(gpio, i2c)->base);
    printf("w = %d , h = %d , format = %d , vendor = %s.\n",
           dsp->info.width,
           dsp->info.height,
           dsp->info.pixel_format,
           dsp->info.vendor);

    size_t size = (dsp->info.width) * (dsp->info.height) * (dsp->info.pixel_format) / 8;
    LOG("need alloc %d buffer", size);

    const char *node = "/dev/fb0";
    int fd = open(node, O_RDWR);
    if (fd < 0) {
        ERROR("Node %s open failed", node);
        exit(1);
    }
    uint8_t *canvas = (uint8_t *) mmap(
            0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0
    );
    if (canvas == MAP_FAILED) {
        ERROR("Node %s mmap failed", node);
        exit(1);
    }
    close(fd);

    uint8_t value = 0xFF;

    dsp->ops.init(dsp);
    dsp->ops.turn_on(dsp);
    dsp->ops.reset(dsp);

    while ((value >>= 1) > 0) {
        memset(canvas, value, size);
        dsp->ops.update(dsp, canvas);
        usleep(250 * 1000);
    }
    munmap(canvas, size);
    delete_BcmGPIO(gpio);
    delete_BcmI2C(i2c);
    delete_Ssd1306Display(container_of(dsp, Ssd1306Display_I2C, base));

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
    return 0;
}