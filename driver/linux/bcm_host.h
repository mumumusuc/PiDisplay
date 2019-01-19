//
// Created by mumumusuc on 19-1-18.
//

#ifndef BCM_HOST_H
#define BCM_HOST_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void bcm_host_init(void);
void bcm_host_deinit(void);

int32_t graphics_get_display_size(const uint16_t, uint32_t *, uint32_t *);

unsigned bcm_host_get_peripheral_address(void);
unsigned bcm_host_get_peripheral_size(void);
unsigned bcm_host_get_sdram_address(void);

#ifdef __cplusplus
};
#endif

#endif //BCM_HOST_H
