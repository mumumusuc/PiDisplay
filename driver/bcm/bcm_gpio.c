//
// Created by mumumusuc on 19-1-19.
//

#include <assert.h>
#include <common.h>
#include "bcm.h"
#include "bcm2835.h"

static void init(void *self, void *info) {
    LOG("bcm_gpio init");
    assert(info);
    assert(bcm2835_init());
    GPIOInfo *_info = (GPIOInfo *) info;
    uint8_t pin = _info->pin;
    uint8_t mode = _info->mode;
    bcm2835_gpio_fsel(pin, mode);
}

static void uninit(void *self) {
    LOG("bcm_gpio uninit");
    bcm2835_close();
}

static int dwrite(void *self, uint8_t *pin, size_t level) {
    //LOG("bcm_gpio write: %d, %d", (*pin), level);
    bcm2835_gpio_write(*pin, (uint8_t) level);
    return 1;
}

static int dread(void *self, uint8_t *buff, size_t pin) {
    //LOG("bcm_gpio read");
    *buff = bcm2835_gpio_lev(pin);
}

BcmGPIO *new_BcmGPIO() {
    BcmGPIO *gpio = (BcmGPIO *) malloc(sizeof(BcmGPIO));
    assert(gpio);
    DriverOps ops = {
            .init = init,
            .uninit = uninit,
            .write = dwrite,
            .read = dread,
    };
    init_GPIO(&(gpio->base), &ops);
    return gpio;
}

void del_BcmGPIO(BcmGPIO *gpio) {
    if (gpio) {
        //del_GPIO(gpio->base);
    }
    free(gpio);
    gpio = NULL;
}