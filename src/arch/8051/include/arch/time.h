// SPDX-License-Identifier: GPL-3.0-only

#ifndef _ARCH_TIME_H
#define _ARCH_TIME_H

#include <stdint.h>

void time_init(void);
uint32_t time_get(void);

// For measuring intervals much smaller than 1 minute, 16-bit time can save some
// bytes of RAM.
inline uint16_t time16_get(void) { return (uint16_t)time_get(); }

#endif // _ARCH_TIME_H
