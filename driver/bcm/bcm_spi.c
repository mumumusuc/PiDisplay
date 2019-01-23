//
// Created by mumumusuc on 19-1-19.
//

#include <assert.h>
#include "common.h"
#include "bcm.h"
#include "bcm2835.h"

#define LOG_TAG     "BCM_SPI"

static void init(void *self, void *info) {
    LOG("init");
    assert(info);
    assert(bcm2835_init());
    SPIInfo *_info = (SPIInfo *) info;
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);        //高位先传输
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                     //spi模式0
    //bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_128);     //分频，
    bcm2835_spi_set_speed_hz(_info->speed);
    bcm2835_spi_chipSelect(_info->cs);                        //设置片选
    bcm2835_spi_setChipSelectPolarity(_info->cs, LOW);        //设置片选低电平有效
    bcm2835_spi_begin();
}

static void uninit(void *self) {
    LOG("uninit");
    bcm2835_spi_end();
    bcm2835_close();
}

static int dwrite(void *self, uint8_t *buf, size_t len) {
    //LOG("write");
    bcm2835_spi_transfern(buf, len);
    //return bcm2835_i2c_write(buf, len);
    return 1;
}

static int dread(void *self, uint8_t *buf, size_t len) {
    //LOG("write");
    return 1;//bcm2835_i2c_read(buf, len);
}

BcmSPI *new_BcmSPI() {
    BcmSPI *spi = (BcmSPI *) malloc(sizeof(BcmSPI));
    DriverOps ops = {
            .init = init,
            .uninit = uninit,
            .write = dwrite,
            .read = dread,
    };
    init_SPI(&(spi->base), &ops);
    return spi;
}

void del_BcmSPI(BcmSPI *spi) {
    free(spi);
    spi = NULL;
}