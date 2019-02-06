//
// Created by mumumusuc on 19-2-1.
//

#include "bcm2835_gpio.h"
#include <asm/types.h>
#include <asm/io.h>

// define bcm_host apis
#if defined(PI_ZERO) || defined(PI_ZERO_W) || defined(PI_1) || defined(CM)
#define PERIPHERAL_ADDRESS          0x20000000
#define SDRAM_ADDRESS               0x40000000
#elif   defined(PI_2) || defined(PI_3) || defined(CM_3)
#define PERIPHERAL_ADDRESS          0x3F000000
#define SDRAM_ADDRESS               0xC0000000
#endif
#define PERIPHERAL_SIZE             0x01000000

// define bcm registers
#define REG_SIZE            sizeof(u32)
#define GPIO_BASE_ADDR      (PERIPHERAL_ADDRESS + 0x200000)
#define GPIO_FSEL_ADDR      (GPIO_BASE_ADDR + 0x0000)
#define GPIO_SET_ADDR       (GPIO_BASE_ADDR + 0x001C)
#define GPIO_CLR_ADDR       (GPIO_BASE_ADDR + 0x0028)
#define GPIO_LEV_ADDR       (GPIO_BASE_ADDR + 0x0034)

#define MAP_FAILED ((u32 *) 0)
static volatile u32 *bcm_gpio_base = MAP_FAILED;
static volatile u32 *bcm_gpio_fsel = MAP_FAILED;
static volatile u32 *bcm_gpio_set = MAP_FAILED;
static volatile u32 *bcm_gpio_clr = MAP_FAILED;
static volatile u32 *bcm_gpio_lev = MAP_FAILED;

#define GPIO_FSEL(n)      (bcm_gpio_fsel + (n))
#define GPIO_SET(n)       (bcm_gpio_set + (n))
#define GPIO_CLR(n)       (bcm_gpio_clr + (n))
#define GPIO_LEV(n)       (bcm_gpio_lev + (n))

static inline void write_address(volatile u32 *address, u32 value) {
    __sync_synchronize();
    *address = value;
    __sync_synchronize();
}

static inline u32 read_addr(volatile u32 *address) {
    u32 value;
    __sync_synchronize();
    value = *address;
    __sync_synchronize();
    return value;
}

inline int gpio_init(void) {
    if (bcm_gpio_base != MAP_FAILED) {
        return -1;
    }
    bcm_gpio_base = (volatile u32 *) ioremap(GPIO_BASE_ADDR, REG_SIZE * 1);
    bcm_gpio_fsel = (volatile u32 *) ioremap(GPIO_FSEL_ADDR, REG_SIZE * 6);
    bcm_gpio_set = (volatile u32 *) ioremap(GPIO_SET_ADDR, REG_SIZE * 2);
    bcm_gpio_clr = (volatile u32 *) ioremap(GPIO_CLR_ADDR, REG_SIZE * 2);
    bcm_gpio_lev = (volatile u32 *) ioremap(GPIO_LEV_ADDR, REG_SIZE * 2);
    return 0;
}

inline int gpio_deinit(void) {
    if (bcm_gpio_base == MAP_FAILED) {
        return -1;
    }
    iounmap(bcm_gpio_base);
    iounmap(bcm_gpio_fsel);
    iounmap(bcm_gpio_set);
    iounmap(bcm_gpio_clr);
    iounmap(bcm_gpio_lev);
    return 0;
}

inline void gpio_set_mode(u8 pin, u8 mode) {
    u8 offset = pin / 10;
    u8 shift = 3 * (pin % 10);
    volatile u32 *addr = GPIO_FSEL(offset);
    u32 value = read_addr(addr);
    u32 mask = ~(0x00000007 << shift);
    value = (value & mask) | ((mode & 0x07) << shift);
    write_address(addr, value);
}

inline u8 gpio_get_mode(u8 pin) {
    u8 offset = pin / 10;
    u8 shift = 3 * (pin % 10);
    volatile u32 *addr = GPIO_FSEL(offset);
    u32 value = read_addr(addr);
    u32 mask = 0x00000007 << shift;
    return 0xFF & ((value & mask) >> shift);
}

inline void gpio_set_value(u8 pin, u8 level) {
    u8 lev = level & 0x01;
    u8 offset = pin / 32;
    u8 shift = pin % 32;
    volatile u32 *addr = (lev == GPIO_HIGH) ? GPIO_SET(offset) : GPIO_CLR(offset);
    u32 value = read_addr(addr);
    u32 mask = (0x00000001 << shift);
    value = (value & ~mask) | mask;
    write_address(addr, value);
}

inline u8 gpio_get_value(u8 pin) {
    u8 offset = pin / 32;
    u8 shift = pin % 32;
    volatile u32 *addr = GPIO_LEV(offset);
    u32 value = read_addr(addr);
    u32 mask = (0x00000001 << shift);
    return 0xFF & ((value & mask) >> shift);
}