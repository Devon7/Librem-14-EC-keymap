// SPDX-License-Identifier: GPL-3.0-only

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <ec/wdt.h>
#include <board/wdt.h>


/*
 * 0 = ~14ms
 * 1 = ~114ms
 * 2 = ~810ms
 * 3 = ~7294ms
 */
void wdt_init(uint8_t timeout)
{
    if (timeout > 3)
        timeout = 3;

    CKCON |= (timeout << 6);
    WDTCON &= ~WDTEN;
}

void wdt_enable(void)
{
    WDTCON |= WDTEN;
    wdt_trigger();
}

void wdt_disable(void)
{
    wdt_trigger();
    WDTCON &= ~WDTEN;
}

void wdt_trigger(void)
{
    WDTCON |= WDTRST;
    WDTCON &= ~WDTRST;
}
