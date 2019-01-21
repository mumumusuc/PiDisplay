//
// Created by mumumusuc on 19-1-19.
//

#include <stdlib.h>
#include <assert.h>
#include "common.h"
#include "bcm.h"
#include "bcm2835.h"

static void init(void *info) {
    LOG("bcm_i2c init");
    assert(info);
    int ref = bcm2835_init();
    LOG("init bcm = %d", ref);
    assert(ref);
    I2CInfo *_info = (I2CInfo *) info;
    uint8_t address = _info->address;
    size_t baudrate = _info->baudrate;
    bcm2835_i2c_set_baudrate(baudrate);
    bcm2835_i2c_setSlaveAddress(address);
    bcm2835_i2c_begin();
}

static void uninit() {
    LOG("bcm_i2c uninit");
    bcm2835_i2c_end();
    bcm2835_close();
}

static int write(uint8_t *buf, size_t len) {
    return bcm2835_i2c_write(buf, len);
}

static int read(uint8_t *buf, size_t len) {
    return bcm2835_i2c_read(buf, len);
}

BcmI2C *new_BcmI2C() {
    BcmI2C *i2c = (BcmI2C *) malloc(sizeof(BcmI2C));
    i2c->base.base.init = init;
    i2c->base.base.uninit = uninit;
    i2c->base.base.write = write;
    i2c->base.base.read = read;
    return i2c;
}

void delete_BcmI2C(BcmI2C *i2c) {
    free(i2c);
    i2c = NULL;
}