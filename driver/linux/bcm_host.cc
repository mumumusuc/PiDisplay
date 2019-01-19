//
// Created by mumumusuc on 19-1-18.
//

#include "bcm_host.h"

#define PI_3

#if defined(PI_ZERO) || defined(PI_ZERO_W) || defined(PI_1) || defined(CM)
#define PERIPHERAL_ADDRESS          0x20000000
#define SDRAM_ADDRESS               0x40000000
#elif   defined(PI_2) || defined(PI_3) || defined(CM_3)
#define PERIPHERAL_ADDRESS          0x3F000000
#define SDRAM_ADDRESS               0xC0000000
#endif

#define PERIPHERAL_SIZE             0x01000000

//void bcm_host_init(void){}
//void bcm_host_deinit(void){}
//int32_t graphics_get_display_size(const uint16_t, uint32_t *, uint32_t *){}

unsigned bcm_host_get_peripheral_address(void) {
    return PERIPHERAL_ADDRESS;
}

unsigned bcm_host_get_peripheral_size(void) {
    return PERIPHERAL_SIZE;
}

unsigned bcm_host_get_sdram_address(void) {
    return SDRAM_ADDRESS;
}