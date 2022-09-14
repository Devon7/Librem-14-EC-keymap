// SPDX-License-Identifier: GPL-3.0-only
//
// TI BQ24715 Smart Charger
// https://www.ti.com/lit/gpn/BQ24715

#include <board/battery.h>
#include <board/smbus.h>
#include <common/debug.h>

#include <charger/bq24715.h>


static bool charger_enabled = true; // charger is enable by default at POR
static int last_current = 0;

int battery_charger_disable(void) {
    int res = 0;

    if (!charger_enabled) {
        return 1;
    }

    DEBUG("CHG disable\n");

    // Set charge option 0 with 175s watchdog
    res = smbus_write(
        CHARGER_ADDRESS,
        0x12,
        SBC_CHARGE_INHIBIT | SBC_LSFET_OCP_THR | SBC_PWM_FREQ_1MHZ | SBC_FIX_IOUT
//        SBC_CHARGE_INHIBIT | SBC_LDO_MODE_EN | SBC_LSFET_OCP_THR | SBC_PWM_FREQ_800KHZ | SBC_WDTMR_ADJ_175S
//        SBC_EN_LWPWR | SBC_CHARGE_INHIBIT | SBC_LDO_MODE_EN | SBC_LSFET_OCP_THR | SBC_PWM_FREQ_1MHZ | SBC_WDTMR_ADJ_175S | SBC_IDPM_EN
    );
    if (res < 0) {
        DEBUG("CHG disable failed!\n");
        return res;
    }

    DEBUG("CHG disabled\n");

    charger_enabled = false;
    last_current = 0;

    battery_charger_debug();

    return 0;
}

int battery_charger_set_charge_current(int current)
{
    int res;

    // Set charge current in mA,
    if (current == last_current)
        return 0;

    // Set charge voltage in mV,
    // must be set before charge current
    res = smbus_write(CHARGER_ADDRESS, 0x15, battery_charge_voltage & 0x7FF0);
    if (res < 0)
        return res;

    res = smbus_write(CHARGER_ADDRESS, 0x14, current & 0x1FC0);
    if (res < 0) {
        return res;
    } else {
        last_current = current;
        return 0;
    }
}

int battery_charger_enable(void) {
    int res = 0;
    
    if (charger_enabled) {
        if (battery_charge_current != last_current) {
            res = battery_charger_set_charge_current(battery_charge_current);
            return res;
        } else
            return 1;
    }

    DEBUG("CHG enable @ %dmV %dmA\n", battery_charge_voltage, battery_charge_current);

    // first make sure charge is inhibited before changing parameters
    res = smbus_write(
        CHARGER_ADDRESS,
        0x12,
        // SBC_CHARGE_INHIBIT | SBC_LDO_MODE_EN | SBC_LSFET_OCP_THR | SBC_PWM_FREQ_1MHZ | SBC_WDTMR_ADJ_175S | SBC_IDPM_EN
        SBC_CHARGE_INHIBIT | SBC_LSFET_OCP_THR | SBC_PWM_FREQ_1MHZ | SBC_FIX_IOUT
    );
    // res = battery_charger_disable();
    if (res < 0)
        return res;

    // Set minimum system voltage
    res = smbus_write(CHARGER_ADDRESS, 0x3E, charger_min_system_voltage & 0x3F00);
    if (res < 0)
        return res;

    // Set input current in mA
    res = smbus_write(CHARGER_ADDRESS, 0x3F, charger_input_current & 0x1FC0);
    if (res < 0)
        return res;

    // Set charge current in mA
    res = battery_charger_set_charge_current(battery_charge_current & 0x1FC0);
    if (res < 0)
        return res;

    // Set charge option 0 with watchdog disabled
    res = smbus_write(
        CHARGER_ADDRESS,
        0x12,
        SBC_PWM_FREQ_1MHZ |
        /*SBC_IDPM_EN |
        SBC_LDO_MODE_EN |*/
        SBC_FIX_IOUT |
        SBC_AUDIO_FREQ_LIM /*|
        SBC_LSFET_OCP_THR */
    );

    DEBUG("CHG enabled\n");
    charger_enabled = true;
    battery_charger_debug();

    return 0;
}

bool battery_charger_is_enabled(void) {
    return charger_enabled;
}

void battery_charger_debug(void) {
    uint16_t data = 0;
    int res = 0;

    #define commandx(N, A, V) { \
        DEBUG("  " #N ": "); \
        res = smbus_read(A, V, &data); \
        if (res < 0) { \
            DEBUG("ERROR %04X\n", -res); \
        } else { \
            DEBUG("%04X\n", data); \
        } \
    }

    #define command(N, A, V) { \
        DEBUG("  " #N ": "); \
        res = smbus_read(A, V, &data); \
        if (res < 0) { \
            DEBUG("ERROR %04X\n", -res); \
        } else { \
            DEBUG("%d\n", data); \
        } \
    }

    DEBUG("Charger (bq24715):\n");
    commandx(ChargeOption0, CHARGER_ADDRESS, 0x12);
    command(ChargeCurrent, CHARGER_ADDRESS, 0x14);
    command(ChargeVoltage, CHARGER_ADDRESS, 0x15);
    command(MinSysVoltage, CHARGER_ADDRESS, 0x3E);
    command(InputCurrent, CHARGER_ADDRESS, 0x3F);
    commandx(ManufacturerID, CHARGER_ADDRESS, 0xFE);
    commandx(DeviceID, CHARGER_ADDRESS, 0xFF);

    #undef command
}

void battery_charger_event(void) {
    //TODO: watchdog
    //DEBUG("CHG event\n");
    //battery_charger_debug();
}

