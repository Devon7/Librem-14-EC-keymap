// SPDX-License-Identifier: GPL-3.0-only

#ifndef _EC_WDT_H
#define _EC_WDT_H

volatile __sfr __at(0x008e) CKCON;
#define WDTCNT17	(0x00 << 6) // ~14ms
#define WDTCNT20	(0x01 << 6) // ~114ms
#define WDTCNT23	(0x02 << 6) // ~810ms
#define WDTCNT26	(0x03 << 6) // ~7294ms

volatile __sfr __at(0x00d8) WDTCON;
#define WDTRST		(1 << 0)
#define WDTEN		(1 << 1)

#endif
