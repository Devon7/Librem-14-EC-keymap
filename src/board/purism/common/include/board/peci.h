// SPDX-License-Identifier: GPL-3.0-only

#ifndef _BOARD_PECI_H
#define _BOARD_PECI_H

#include <ec/peci.h>

extern int16_t peci_temp;

void peci_init(void);
int peci_wr_pkg_config(uint8_t index, uint16_t param, uint32_t data);
int peci_update_PL1(int watt);
int peci_update_PL2(int watt);
int peci_update_PsysPL2(int watt);
int peci_update_PL4(int watt);
void peci_event(void);

#endif // _BOARD_PECI_H
