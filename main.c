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
#include <assert.h>
#include "common.h"
#include "device/ssd1306/ssd1306.h"
#include "driver/bcm/bcm.h"

int main(int argc, char *argv[]) {
    printf("Hello world.\n");
    printf("Sizeof(Display) = %d.\n", sizeof(Ssd1306_I2C));


    BcmGPIO *bcm_gpio = new_BcmGPIO();
    BcmGPIO *bcm_gpio2 = new_BcmGPIO();
    BcmI2C *bcm_i2c = new_BcmI2C();
    BcmSPI *bcm_spi = new_BcmSPI();
    Ssd1306_I2C *ssd1306_i2c = new_Ssd1306_I2C(&(bcm_gpio->base), &(bcm_i2c->base));
    Ssd1306_SPI4 *ssd1306_spi = new_Ssd1306_SPI4(&(bcm_gpio2->base), &(bcm_spi->base));
    Display *dsps[] = {
            &(ssd1306_i2c->base.base),
            &(ssd1306_spi->base.base)
    };
    size_t l_dsp = 2;
    Display *dsp = dsps[1];//&(ssd1306_i2c->base.base);
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

    for (int i = 0; i < l_dsp; i++) {
        dsp = dsps[i];
        dsp->ops.begin(dsp);
        dsp->ops.reset(dsp);
        dsp->ops.turn_on(dsp);
        //dsp->ops.clear(dsp);
    }

    while ((value >>= 1) > 0) {
        memset(canvas, value, size);
        dsps[0]->ops.update(dsps[0], canvas);
        dsps[1]->ops.update(dsps[1], canvas);
        usleep(200 * 1000);
    }
    memset(canvas, 0, size);
    dsps[0]->ops.update(dsps[0], canvas);
    dsps[1]->ops.update(dsps[1], canvas);
    munmap(canvas, size);
    for (int i = 0; i < l_dsp; i++) {
        dsp = dsps[i];
        dsp->ops.clear(dsp);
        dsp->ops.turn_off(dsp);
        dsp->ops.end(dsp);
    }
    del_BcmGPIO(bcm_gpio);
    del_BcmI2C(bcm_i2c);
    del_BcmSPI(bcm_spi);
    del_Ssd1306_I2C(ssd1306_i2c);
    del_Ssd1306_SPI4(ssd1306_spi);
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