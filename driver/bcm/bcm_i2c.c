//
// Created by mumumusuc on 19-1-19.
//

#include <stdlib.h>
#include <assert.h>
#include "common.h"
#include "bcm.h"
#include "bcm2835.h"

static void init(void *self, void *info) {
    LOG("bcm_i2c init");
    assert(info);
    assert(bcm2835_init());
    I2CInfo *_info = (I2CInfo *) info;
    bcm2835_i2c_set_baudrate(_info->baudrate);
    bcm2835_i2c_setSlaveAddress(_info->address);
    bcm2835_i2c_begin();
}

static void uninit(void *self) {
    LOG("bcm_i2c uninit");
    bcm2835_i2c_end();
    bcm2835_close();
}

static int dwrite(void *self, uint8_t *buf, size_t len) {
    //LOG("bcm_i2c write");
    return bcm2835_i2c_write(buf, len);
}

static int dread(void *self, uint8_t *buf, size_t len) {
    //LOG("bcm_i2c write");
    return bcm2835_i2c_read(buf, len);
}

BcmI2C *new_BcmI2C() {
    BcmI2C *i2c = (BcmI2C *) malloc(sizeof(BcmI2C));
    //i2c->base = new_I2C();
    DriverOps ops = {
            .init = init,
            .uninit = uninit,
            .write = dwrite,
            .read = dread,
    };
    init_I2C(&(i2c->base), &ops);
    return i2c;
}

void del_BcmI2C(BcmI2C *i2c) {
    if (i2c) {
       // del_I2C(&(i2c->base));
    }
    free(i2c);
    i2c = NULL;
}