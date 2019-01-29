//
// Created by mumumusuc on 19-1-19.
//

#include <assert.h>
#include "common.h"
#include "driver_protected.h"
#include "bcm.h"
#include "bcm2835.h"

#undef  LOG_TAG
#define LOG_TAG     "BCM_SPI"

static void _begin(Spi *self) {
    LOG("%s", __func__);
    assert(bcm2835_init());
}

static void _init(Spi *self, const SpiInfo *info) {
    LOG("%s", __func__);
    assert(info);
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);        //高位先传输
    bcm2835_spi_setDataMode(info->mode);                     //spi模式0
    //bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_128);     //分频，
    bcm2835_spi_set_speed_hz(info->speed);
    bcm2835_spi_chipSelect(info->cs);                        //设置片选
    bcm2835_spi_setChipSelectPolarity(info->cs, LOW);        //设置片选低电平有效
    bcm2835_spi_begin();
}

static void _write(Spi *self, const uint8_t *buf, size_t len) {
    //LOG("%s",__func__);
    bcm2835_spi_transfern(buf, len);
}

static void _read(Spi *self, uint8_t *buf, size_t len) {
    //LOG("%s",__func__);
    //bcm2835_i2c_read(buf, len);
}

static void _end(Spi *self) {
    LOG("%s", __func__);
    bcm2835_spi_end();
    bcm2835_close();
}

static SpiVTbl _vtbl = {
        .begin = _begin,
        .init = _init,
        .read = _read,
        .write = _write,
        .end = _end,
};

BcmSpi *new_bcm_spi() {
    BcmSpi *spi = (BcmSpi *) malloc(sizeof(BcmSpi));
    assert(spi);
    Spi *super = new_spi();
    super->vtbl = &_vtbl;
    link2(spi, super, "BCM_SPI", del_bcm_spi);
    return spi;
}

void del_bcm_spi(void *spi) {
    LOG("%s", __func__);
    if (spi) {
        BcmSpi *self = (BcmSpi *) spi;
        object_delete(self->obj);
    }
    free(spi);
    spi = NULL;
}