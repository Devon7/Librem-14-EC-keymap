// SPDX-License-Identifier: GPL-3.0-only

#ifndef _BOARD_BATTERY_H
#define _BOARD_BATTERY_H

#include <stdbool.h>
#include <stdint.h>

#ifndef BATTERY_ADDRESS
    #define BATTERY_ADDRESS 0x0B
#endif

#ifndef CHARGER_ADDRESS
    #define CHARGER_ADDRESS 0x09
#endif

/* SBS status register */
#define BATTERY_OVER_CHARGED_ALARM        (1U << 15)
#define BATTERY_TERMINATE_CHARGE_ALARM    (1U << 14)
#define BATTERY_OVER_TEMP_ALARM           (1U << 12)
#define BATTERY_TERMINATE_DISCHARGE_ALARM (1U << 11)
#define BATTERY_REMAINING_CAPACITY_ALARM  (1U << 9)
#define BATTERY_REMAINING_TIME_ALARM      (1U << 8)
#define BATTERY_INITIALIZED               (1U << 7)
#define BATTERY_DISCHARGING               (1U << 6)
#define BATTERY_FULLY_CHARGED             (1U << 5)
#define BATTERY_FULLY_DISCHARGED          (1U << 4)

#define BATTERY_OK                        (0x0000)
#define BATTERY_BUSY                      (0x0001)
#define BATTERY_RESERVED_COMMAND          (0x0002)
#define BATTERY_UNSUPPORTED_COMMAND       (0x0003)
#define BATTERY_ACCESS_DENIED             (0x0004)
#define BATTERY_OVER_UNDER_FLOW           (0x0005)
#define BATTERY_BAD_SIZE                  (0x0006)
#define BATTERY_UNKNOWN_ERROR             (0x0007)

extern uint16_t battery_temp;
extern uint16_t battery_voltage;
extern uint16_t battery_current;
extern uint16_t battery_charge;
extern uint16_t battery_remaining_capacity;
extern uint16_t battery_full_capacity;
extern uint16_t battery_status;
extern uint16_t battery_design_capacity;
extern uint16_t battery_design_voltage;
extern uint16_t battery_charge_voltage;
extern uint16_t battery_charge_current;
extern uint16_t battery_charge_max_current;
extern uint16_t battery_min_voltage;
extern uint16_t battery_cycle_count;
extern uint16_t battery_manufacturing_date;

extern bool battery_present;

extern uint16_t charger_input_current;
extern uint16_t charger_min_system_voltage;

uint8_t battery_get_start_threshold(void);
bool battery_set_start_threshold(uint8_t value);

uint8_t battery_get_end_threshold(void);
bool battery_set_end_threshold(uint8_t value);

void battery_event(void);
void battery_reset(void);

// Defined by charger/*.c
int battery_charger_disable(void);
int battery_charger_set_charge_current(int current);
int battery_charger_enable(void);
bool battery_charger_is_enabled(void);
void battery_charger_event(void);
void battery_charger_debug(void);

#endif // _BOARD_BATTERY_H
