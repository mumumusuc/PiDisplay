//
// Created by mumumusuc on 19-1-19.
//

#include <string.h>
#include<fcntl.h>   //open()
#include<unistd.h>  //close()
#include<errno.h>   //errno
#include<sys/mman.h> //mmap()
#include <assert.h>

#include "common.h"
#include "factory.h"


int main(int argc, char *argv[]) {
    /*
    Display *disp = create_display(DISPLAY_NONE);
    void *buffer = malloc(1);
    printf("w = %ld , h = %ld , format = %d , vendor = %s. \n",
           disp->info.width,
           disp->info.height,
           disp->info.pixel_format,
           disp->info.vendor);
    disp->ops.begin(disp);
    disp->ops.reset(disp);
    disp->ops.turn_on(disp);
    disp->ops.clear(disp);
    disp->ops.update(disp, buffer);
    disp->ops.turn_off(disp);
    disp->ops.end(disp);
    free(buffer);
    destroy_display(disp);
    */
    Display *dsps[] = {
            create_display(DISPLAY_SSD1306_BCM_I2C),
            create_display(DISPLAY_SSD1306_BCM_SPI4),
    };
    size_t l_dsp = 2;
    Display *dsp = dsps[0];

    printf("w = %ld , h = %ld , format = %d , vendor = %s , l_dsp = %ld. \n",
           dsp->info.width,
           dsp->info.height,
           dsp->info.pixel_format,
           dsp->info.vendor,
           l_dsp);
    size_t size = (dsp->info.width) * (dsp->info.height) * (dsp->info.pixel_format) / 8;
    LOG("need alloc %ld buffer", size);

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

    for (int i = 0; i < l_dsp; i++) {
        dsp = dsps[i];
        dsp->ops.begin(dsp);
        dsp->ops.reset(dsp);
        dsp->ops.turn_on(dsp);
        dsp->ops.clear(dsp);
    }

    while ((value >>= 1) > 0) {
        memset(canvas, value, size);
        for (int i = 0; i < l_dsp; i++) {
            dsp = dsps[i];
            dsp->ops.update(dsp, canvas);
            dsp->ops.update(dsp, canvas);
        }
        usleep(200 * 1000);
    }
    memset(canvas, 0, size);
    dsps[0]->ops.update(dsps[0], canvas);
    dsps[1]->ops.update(dsps[1], canvas);
    munmap(canvas, size);
    for (int i = 0; i < l_dsp; i++) {
        dsp = dsps[i];
        printf("clear %d.\n", i);
        dsp->ops.clear(dsp);
        printf("clear %d end.\n", i);
        dsp->ops.turn_off(dsp);
        dsp->ops.end(dsp);
    }
    for (int i = 0; i < l_dsp; i++) {
        dsp = dsps[i];
        destroy_display(dsp);
    }
/*
    BcmGPIO *bcm_gpio = new_BcmGPIO();
    GPIO *gpio = bcm_gpio->base;
    GPIOInfo info = {
            .pin=21,
            .mode=1,
    };
    gpio->ops.init(gpio, &info);
    int i = 5;
    while (i-- > 0) {
        gpio->ops.write(gpio, &(info.pin), 1);
        usleep(500 * 1000);
        gpio->ops.write(gpio, &(info.pin), 0);
        usleep(500 * 1000);
    }
    gpio->ops.uninit(gpio);
    del_BcmGPIO(bcm_gpio);
    return 0;
*/
}