//
// Created by mumumusuc on 19-1-19.
//

#include <assert.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include "driver_protected.h"
#include "default.h"

#undef  LOG_TAG
#define LOG_TAG     "DEFAULT_I2C"

static const char *_node = "/dev/i2c-1";

struct _I2cPriv {
    int fd;
};

static void _begin(I2c *self) {
    LOG("%s", __func__);
    DefaultI2c *i2c = subclass(self, DefaultI2c);
    i2c->priv->fd = open(_node, O_RDWR);
    if (i2c->priv->fd < 0) {
        ERROR("Error on opening %s", _node);
        exit(1);
    }
}

static void _init(I2c *self, const I2cInfo *info) {
    LOG("%s", __func__);
    assert(info);
    DefaultI2c *i2c = subclass(self, DefaultI2c);
    int _fd = i2c->priv->fd;
    ioctl(_fd, I2C_TIMEOUT, 2);
    ioctl(_fd, I2C_RETRIES, 1);
    ioctl(_fd, I2C_TENBIT, 0);
    if (ioctl(_fd, I2C_SLAVE, info->address) < 0) {
        ERROR("Init slave failed");
        exit(1);
    }
}

static void _write(I2c *self, const uint8_t *buf, size_t len) {
    //LOG("%s",__func__);
    write(subclass(self, DefaultI2c)->priv->fd, buf, len);
}

static void _read(I2c *self, uint8_t *buf, size_t len) {
    //LOG("%s",__func__);
    METHOD_NOT_IMPLEMENTED();
}

static void _end(I2c *self) {
    LOG("%s", __func__);
    close(subclass(self, DefaultI2c)->priv->fd);
}

static I2cVTbl _vtbl = {
        .begin = _begin,
        .init = _init,
        .read = _read,
        .write = _write,
        .end = _end,
};

DefaultI2c *new_default_i2c() {
    LOG("%s", __func__);
    DefaultI2c *i2c = (DefaultI2c *) malloc(sizeof(DefaultI2c));
    assert(i2c);
    i2c->priv = (I2cPriv *) malloc(sizeof(I2cPriv));
    I2c *super = new_i2c();
    super->vtbl = &_vtbl;
    extend(i2c, super, "DEFAULT_I2C", del_default_i2c);
    return i2c;
}

void del_default_i2c(void *i2c) {
    LOG("%s", __func__);
    if (i2c) {
        DefaultI2c *self = (DefaultI2c *) i2c;
        object_delete(object(self));
    }
    free(i2c);
    i2c = NULL;
}