//
// Created by mumumusuc on 19-1-19.
//

#include <assert.h>
#include "driver_protected.h"
#include "bcm_.h"
#include "bcm2835.h"

#undef  LOG_TAG
#define LOG_TAG     "BCM_GPIO"

static void _begin(Gpio *self) {
    LOG("%s", __func__);
    assert(bcm2835_init());
}

static void _init(Gpio *self, const GpioInfo *info) {
    LOG("%s", __func__);
    assert(info);
    bcm2835_gpio_fsel(info->pin, info->mode);
}

static void _write(void *self, const uint8_t *const pin, const size_t level) {
    bcm2835_gpio_write(*pin, (uint8_t) level);
}

static void _read(void *self, uint8_t *const buff, const size_t pin) {
    *buff = bcm2835_gpio_lev(pin);
}

static void _end(void *self) {
    LOG("%s", __func__);
    assert(bcm2835_close());
}

static GpioVTbl _vtbl = {
        .begin = _begin,
        .init = _init,
        .read = _read,
        .write = _write,
        .end = _end,
};

BcmGpio *new_bcm_gpio() {
    BcmGpio *gpio = (BcmGpio *) malloc(sizeof(BcmGpio));
    assert(gpio);
    Gpio *super = new_gpio();
    super->vtbl = &_vtbl;
    link2(gpio, super, LOG_TAG, del_bcm_gpio);
    return gpio;
}

void del_bcm_gpio(void *gpio) {
    if (gpio) {
        BcmGpio *self = (BcmGpio *) gpio;
        object_delete(self->obj);
    }
    free(gpio);
    gpio = NULL;
}