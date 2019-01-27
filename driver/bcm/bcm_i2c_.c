//
// Created by mumumusuc on 19-1-27.
//

#include <assert.h>
#include "driver_protected.h"
#include "bcm_.h"
#include "bcm2835.h"

#undef  LOG_TAG
#define LOG_TAG     "BCM_I2C"

static void _begin(I2c *self) {
    LOG("%s", __func__);
    assert(bcm2835_init());
}

static void _init(I2c *self, const GpioInfo *info) {
    LOG("%s", __func__);
    assert(info);
    I2cInfo *_info = (I2cInfo *) info;
    bcm2835_i2c_set_baudrate(_info->baudrate);
    bcm2835_i2c_setSlaveAddress(_info->address);
    bcm2835_i2c_begin();
}

static void _write(I2c *self, const uint8_t *buf, size_t len) {
    //LOG("%s",__func__);
    bcm2835_i2c_write(buf, len);
}

static void _read(I2c *self, uint8_t *buf, size_t len) {
    //LOG("%s",__func__);
    bcm2835_i2c_read(buf, len);
}

static void _end(I2c *self) {
    LOG("%s", __func__);
    bcm2835_i2c_end();
    assert(bcm2835_close());
}

static I2cVTbl _vtbl = {
        .begin = _begin,
        .init = _init,
        .read = _read,
        .write = _write,
        .end = _end,
};

BcmI2c *new_bcm_i2c() {
    LOG("%s", __func__);
    BcmI2c *i2c = (BcmI2c *) malloc(sizeof(BcmI2c));
    assert(i2c);
    I2c *super = new_i2c();
    super->vtbl = &_vtbl;
    link2(i2c, super, "BCM_I2C", del_bcm_i2c);
    return i2c;
}

void del_bcm_i2c(void *i2c) {
    LOG("%s", __func__);
    if (i2c) {
        BcmI2c *self = (BcmI2c *) i2c;
        object_delete(self->obj);
    }
    free(i2c);
    i2c = NULL;
}