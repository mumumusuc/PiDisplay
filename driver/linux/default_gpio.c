//
// Created by mumumusuc on 19-1-19.
//

#include <assert.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "driver_protected.h"
#include "default.h"

#undef  LOG_TAG
#define LOG_TAG     "DEFAULT_GPIO"

// define bcm_host apis
#if defined(PI_ZERO) || defined(PI_ZERO_W) || defined(PI_1) || defined(CM)
#define PERIPHERAL_ADDRESS          0x20000000
#define SDRAM_ADDRESS               0x40000000
#elif   defined(PI_2) || defined(PI_3) || defined(CM_3)
#define PERIPHERAL_ADDRESS          0x3F000000
#define SDRAM_ADDRESS               0xC0000000
#endif
#define PERIPHERAL_SIZE             0x01000000

extern unsigned bcm_host_get_peripheral_address(void) API_WEAK;

extern unsigned bcm_host_get_peripheral_size(void) API_WEAK;

extern unsigned bcm_host_get_sdram_address(void) API_WEAK;

static unsigned get_peripheral_address(void) {
    if (bcm_host_get_peripheral_address)
        return bcm_host_get_peripheral_address();
    return PERIPHERAL_ADDRESS;
}

static unsigned get_peripheral_size(void) {
    if (bcm_host_get_peripheral_size)
        return bcm_host_get_peripheral_size();
    return PERIPHERAL_SIZE;
}

static unsigned get_sdram_address(void) {
    if (bcm_host_get_sdram_address)
        return bcm_host_get_sdram_address();
    return SDRAM_ADDRESS;
}
// end define bcm_host apis

struct _GpioPriv {
    volatile uint32_t *bcm_peripherals_base;
    volatile uint32_t *bcm_gpio_base;
    volatile uint32_t *GPFSEL0;
    volatile uint32_t *GPSET0;
    volatile uint32_t *GPCLR0;
    volatile uint32_t *GPLEV0;
};

static const char *_node = "/dev/mem";

static void write_addr(DefaultGpio *gpio, volatile uint32_t *addr, uint32_t value) {
    __sync_synchronize();
    *addr = value;
    __sync_synchronize();
}

static uint32_t read_addr(DefaultGpio *gpio, volatile uint32_t *addr) {
    uint32_t value;
    __sync_synchronize();
    value = *addr;
    __sync_synchronize();
    return value;
}

static void set_mode(DefaultGpio *gpio, uint8_t pin, uint8_t mode) {
    uint8_t offset = pin / 10;
    uint8_t _pin = pin % 10;
    volatile uint32_t *addr = gpio->priv->GPFSEL0 + offset;
    uint32_t value = read_addr(gpio, addr);
    uint32_t mask = ~(0x00000007 << (3 * _pin));
    value = (value & mask) | ((mode & 0x07) << (3 * _pin));
    write_addr(gpio, addr, value);
}

static void set_value(DefaultGpio *gpio, uint8_t pin, uint8_t level) {
    volatile uint32_t *reg;
    if (level == GPIO_HIGH) {
        reg = gpio->priv->GPSET0;
    } else if (level == GPIO_LOW) {
        reg = gpio->priv->GPCLR0;
    } else {
        ERROR("%s : Undef value", __func__);
        return;
    }
    uint8_t offset = pin / 32;
    uint8_t _pin = pin % 32;
    volatile uint32_t *addr = reg + offset;
    uint32_t value = read_addr(gpio, addr);
    uint32_t mask = (0x00000001 << _pin);
    value = (value & ~mask) | mask;
    write_addr(gpio, addr, value);
}

static void _begin(Gpio *self) {
    LOG("%s", __func__);
    DefaultGpio *gpio = subclass(self, DefaultGpio);
    GpioPriv *priv = gpio->priv;
    if (priv->bcm_peripherals_base != MAP_FAILED) {
        LOG("%s : gpio already began", __func__);
        return;
    }
    int fd = open(_node, O_RDWR | O_SYNC);
    if (fd < 0) {
        ERROR("Can not open %s", _node);
        exit(1);
    }
    const uint32_t per_size = get_peripheral_size();
    const uint32_t per_addr = get_peripheral_address();
    priv->bcm_peripherals_base = (volatile uint32_t *) mmap(
            NULL,
            per_size,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            fd,
            (off_t) per_addr
    );
    if (priv->bcm_peripherals_base == MAP_FAILED) {
        ERROR("Can not mmap %s", _node);
        exit(1);
    }
    uint8_t size = sizeof(uint32_t);
    priv->bcm_gpio_base = priv->bcm_peripherals_base + 0x200000 / size;
    priv->GPFSEL0 = priv->bcm_gpio_base + 0x0000 / size;
    priv->GPSET0 = priv->bcm_gpio_base + 0x001C / size;
    priv->GPCLR0 = priv->bcm_gpio_base + 0x0028 / size;
    priv->GPLEV0 = priv->bcm_gpio_base + 0x0034 / size;
    close(fd);
}

static void _init(Gpio *self, const GpioInfo *info) {
    LOG("%s", __func__);
    assert(info);
    set_mode(subclass(self, DefaultGpio), info->pin, info->mode);
}

static void _write(Gpio *self, const uint8_t *pin, size_t level) {
    set_value(subclass(self, DefaultGpio), *pin, level);
}

static void _read(Gpio *self, uint8_t *buff, size_t pin) {
    DefaultGpio *gpio = subclass(self, DefaultGpio);
    // read mode
    uint8_t offset = pin / 10;
    uint8_t _pin = pin % 10;
    volatile uint32_t *addr = gpio->priv->GPFSEL0 + offset;
    uint32_t value = read_addr(gpio, addr);
    uint32_t mask = 0x00000007 << (3 * _pin);
    uint8_t mode = 0xFF & ((value & mask) >> (3 * _pin));

    offset = pin / 32;
    _pin = pin % 32;
    addr = gpio->priv->GPLEV0 + offset;
    value = read_addr(gpio, addr);
    mask = (0x00000001 << _pin);
    uint8_t level = 0xFF & ((value & mask) >> _pin);

    *buff = ((mode & 0x0F) << 4) | (level & 0x0F);
}

static void _end(Gpio *self) {
    LOG("%s", __func__);
    const uint32_t per_size = get_peripheral_size();
    DefaultGpio *gpio = subclass(self, DefaultGpio);
    munmap((void *) gpio->priv->bcm_peripherals_base, per_size);
}

static GpioVTbl _vtbl = {
        .begin = _begin,
        .init = _init,
        .read = _read,
        .write = _write,
        .end = _end,
};

DefaultGpio *new_default_gpio() {
    LOG("%s", __func__);
    DefaultGpio *gpio = (DefaultGpio *) malloc(sizeof(DefaultGpio));
    assert(gpio);
    gpio->priv = (GpioPriv *) malloc(sizeof(GpioPriv));
    gpio->priv->bcm_peripherals_base = (uint32_t *) MAP_FAILED;
    Gpio *super = new_gpio();
    super->vtbl = &_vtbl;
    extend(gpio, super, "DEFAULT_GPIO", del_default_gpio);
    return gpio;
}

void del_default_gpio(void *gpio) {
    if (gpio) {
        DefaultGpio *self = (DefaultGpio *) gpio;
        free(self->priv);
        self->priv = NULL;
        object_delete(object(self));
    }
    free(gpio);
    gpio = NULL;
}