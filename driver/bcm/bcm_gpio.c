//
// Created by mumumusuc on 19-1-19.
//

#include <assert.h>
#include <common.h>
#include "bcm.h"
#include "bcm2835.h"

static void init(void *info) {
    assert(info);
    bcm2835_init();
    GPIOInfo *_info = (GPIOInfo *) info;
    uint8_t pin = _info->pin;
    uint8_t mode = _info->mode;
    bcm2835_gpio_fsel(pin, mode);
}

static void uninit() {
    LOG("uninit");
    bcm2835_close();
}

static int write(uint8_t *pin, size_t level) {
    LOG("write");
    bcm2835_gpio_write(*pin, (uint8_t) level);
    return 1;
}

static int read(uint8_t *pin, size_t size) {
    return bcm2835_gpio_lev(*pin);
}

BcmGPIO *new_BcmGPIO() {
    BcmGPIO *gpio = NULL;
    gpio = (BcmGPIO *) malloc(sizeof(BcmGPIO));
    assert(gpio);
    gpio->base = (GPIO *) malloc(sizeof(GPIO));
    assert(gpio->base);
    gpio->init = init;
    gpio->uninit = uninit;
    gpio->write = write;
    gpio->read;
    return gpio;
}

void delete_BcmGPIO(BcmGPIO *gpio) {
    if (gpio) {
        free(gpio->base);
        gpio->base = NULL;
    }
    free(gpio);
    gpio = NULL;
}