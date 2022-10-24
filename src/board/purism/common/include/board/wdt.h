// SPDX-License-Identifier: GPL-3.0-only

#ifndef _BOARD_WDT_H
#define _BOARD_WDT_H

/*
 * wdt_init() will setuo the watchdog but not enable it yet
 * timeout:
 * 0 = ~14ms
 * 1 = ~114ms
 * 2 = ~810ms
 * 3 = ~7294ms
 */
void wdt_init(uint8_t timeout);
/* convenience definitions for timeouts */
#define WDT_TMO_17B	(0x00)
#define WDT_TMO_20B	(0x01)
#define WDT_TMO_23B	(0x02)
#define WDT_TMO_26B	(0x03)

/* wdt_enable() enables the watchdog,
   should call wdt_init() first */
void wdt_enable(void);

/* wdt_disable() resets and disables the watchdog */
void wdt_disable(void);

/* wdt_trigger() triggers / resets the watchdog,
   must be called before timeout expires or the EC will reset itself */
void wdt_trigger(void);

#endif
