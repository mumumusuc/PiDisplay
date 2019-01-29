//
// Created by mumumusuc on 19-1-19.
//

#include <assert.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "driver_protected.h"
#include "default.h"

#undef  LOG_TAG
#define LOG_TAG     "DEFAULT_SPI"

static const char *_node[2] = {"/dev/spidev0.0", "/dev/spidev0.1"};

struct _SpiPriv {
    int fd;
};

static void _begin(Spi *self) {
    LOG("%s", __func__);
    LOG("%s end", __func__);
}

static void _init(Spi *self, const SpiInfo *info) {
    LOG("%s", __func__);
    assert(info);
    DefaultSpi *spi = subclass(self, DefaultSpi);
    uint8_t bit_per_word = 8;
    uint8_t mode = info->mode;
    uint8_t channel = info->cs;
    uint32_t speed = info->speed;
    int _fd = open(_node[channel], O_RDWR);
    spi->priv->fd = _fd;
    if (_fd < 0) {
        ERROR("Error on opening %s", _node[channel]);
        exit(errno);
    }
    if (ioctl(_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        ERROR("Can't set spi write mode");
        exit(errno);
    }
    if (ioctl(_fd, SPI_IOC_RD_MODE, &mode) < 0) {
        ERROR("Can't get spi read mode");
        exit(errno);
    }
    if (ioctl(_fd, SPI_IOC_WR_BITS_PER_WORD, &bit_per_word) < 0) {
        ERROR("Can't set bits per word");
        exit(errno);
    }
    if (ioctl(_fd, SPI_IOC_RD_BITS_PER_WORD, &bit_per_word) < 0) {
        ERROR("Can't get bits per word");
        exit(errno);
    }
    if (ioctl(_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        ERROR("Can't set max speed hz");
        exit(errno);
    }

    if (ioctl(_fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0) {
        ERROR("Can't get max speed hz");
        exit(errno);
    }
    LOG("%s end", __func__);
}

static void _write(Spi *self, const uint8_t *buf, size_t len) {
    write(subclass(self, DefaultSpi)->priv->fd, buf, len);
}

static void _read(Spi *self, uint8_t *buf, size_t len) {
    //LOG("%s",__func__);
    METHOD_NOT_IMPLEMENTED();
}

static void _end(Spi *self) {
    LOG("%s", __func__);
    close(subclass(self, DefaultSpi)->priv->fd);
    LOG("%s end", __func__);
}

static SpiVTbl _vtbl = {
        .begin = _begin,
        .init = _init,
        .read = _read,
        .write = _write,
        .end = _end,
};

DefaultSpi *new_default_spi() {
    LOG("%s", __func__);
    DefaultSpi *spi = (DefaultSpi *) malloc(sizeof(DefaultSpi));
    assert(spi);
    spi->priv = (SpiPriv *) malloc(sizeof(SpiPriv));
    Spi *super = new_spi();
    super->vtbl = &_vtbl;
    extend(spi, super, "DEFAULT_SPI", del_default_spi);
    LOG("%s end", __func__);
    return spi;
}

void del_default_spi(void *spi) {
    LOG("%s", __func__);
    if (spi) {
        DefaultSpi *self = (DefaultSpi *) spi;
        object_delete(object(self));
    }
    free(spi);
    spi = NULL;
}