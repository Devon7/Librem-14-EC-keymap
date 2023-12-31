// SPDX-License-Identifier: GPL-3.0-only

#include <board/fan.h>
#include <board/gpio.h>
#include <board/peci.h>
#include <board/power.h>
#include <common/debug.h>
#include <common/macro.h>
#include <ec/espi.h>
#include <ec/gpio.h>
#include <ec/pwm.h>

// Fan speed is the lowest requested over HEATUP seconds
#ifndef BOARD_HEATUP
    #define BOARD_HEATUP 10
#endif

static uint8_t FAN_HEATUP[BOARD_HEATUP] = { 0 };

// Fan speed is the highest HEATUP speed over COOLDOWN seconds
#ifndef BOARD_COOLDOWN
    #define BOARD_COOLDOWN 10
#endif

static uint8_t FAN_COOLDOWN[BOARD_COOLDOWN] = { 0 };

// Tjunction = 100C for i7-10710U (and probably the same for all CML-U)
#define T_JUNCTION 100

int16_t peci_temp = 0;

#define PECI_TEMP(X) (((int16_t)(X)) << 6)

#define FAN_POINT(T, D) { .temp = PECI_TEMP(T), .duty = PWM_DUTY(D) }

// Fan curve with temperature in degrees C, duty cycle in percent
static struct FanPoint __code FAN_POINTS[] = {
#ifdef BOARD_FAN_POINTS
    BOARD_FAN_POINTS
#else
    FAN_POINT(70, 40),
    FAN_POINT(75, 50),
    FAN_POINT(80, 60),
    FAN_POINT(85, 65),
    FAN_POINT(90, 65)
#endif
};

static struct Fan __code FAN = {
    .points = FAN_POINTS,
    .points_size = ARRAY_SIZE(FAN_POINTS),
    .heatup = FAN_HEATUP,
    .heatup_size = ARRAY_SIZE(FAN_HEATUP),
    .cooldown = FAN_COOLDOWN,
    .cooldown_size = ARRAY_SIZE(FAN_COOLDOWN),
    .interpolate = false,
};

void peci_init(void) {
    // Allow PECI pin to be used
    GCR2 |= (1 << 4);

    // Set frequency to 1MHz
    HOCTL2R = HOPTTRS_1MHZ;
    // Set VTT to 1.1V
    PADCTLR = HOVTTS_1_10V;
}

// Returns positive completion code on success, negative completion code or
// negative (0x1000 | status register) on PECI hardware error
int peci_wr_pkg_config(uint8_t index, uint16_t param, uint32_t data) {
    // Wait for completion
    while (HOSTAR & HOBY) {}
    // Clear status
    HOSTAR = HOSTAR;

    // Enable PECI, clearing data fifo's, enable AW_FCS
    HOCTLR = FIFOCLR | PECIHEN | AWFCS_EN;
    // Set address to default
    HOTRADDR = 0x30;
    // Set write length
    HOWRLR = 10;
    // Set read length
    HORDLR = 1;
    // Set command
    HOCMDR = 0xA5;

    // Write host ID
    HOWRDR = 0;
    // Write index
    HOWRDR = index;
    // Write param
    HOWRDR = (uint8_t)param;
    HOWRDR = (uint8_t)(param >> 8);
    // Write data
    HOWRDR = (uint8_t)data;
    HOWRDR = (uint8_t)(data >> 8);
    HOWRDR = (uint8_t)(data >> 16);
    HOWRDR = (uint8_t)(data >> 24);

    // Start transaction
    HOCTLR |= START;

    // Wait for completion
    while (HOSTAR & HOBY) {}

    int status = (int)HOSTAR;
    if (status & FINISH) {
        int cc = (int)HORDDR;
        if (cc & 0x80) {
            return -cc;
        } else {
            return cc;
        }
    } else {
        return -(0x1000 | status);
    }
}

#define PECI_INDEX_POWER_LIMITS_PL1             0x1A
#define PECI_PARAMS_POWER_LIMITS_PL1            0x0000
#define PECI_PL1_CONTROL_TIME_WINDOWS           ((uint32_t)0xDC) /* 28 seconds */
#define PECI_PL1_POWER_LIMIT_ENABLE             (0x01 << 15)
#define PECI_PL1_POWER_LIMIT(x)                 (x << 3)

#define PECI_INDEX_POWER_LIMITS_PL2             0x1B
#define PECI_PARAMS_POWER_LIMITS_PL2            0x0000
#define PECI_PL2_CONTROL_TIME_WINDOWS           (0x00 << 16)
#define PECI_PL2_POWER_LIMIT_ENABLE             (0x01 << 15)
#define PECI_PL2_POWER_LIMIT(x)                 (x << 3)

#define PECI_INDEX_POWER_LIMITS_PSYS_PL2        0x3B
#define PECI_PARAMS_POWER_LIMITS_PSYS_PL2       0x0000
#define PECI_PSYS_PL2_CONTROL_TIME_WINDOWS      ((uint32_t)0xDC << 16) /* 28 seconds */
#define PECI_PSYS_PL2_POWER_LIMIT_ENABLE        (0x01 << 15)
#define PECI_PSYS_PL2_POWER_LIMIT(x)            (x << 3)

#define PECI_INDEX_POWER_LIMITS_PL4             0x3C
#define PECI_PARAMS_POWER_LIMITS_PL4            0x0000
#define PECI_PL4_POWER_LIMIT(x)                 (x << 3)

int peci_update_PL1(int watt)
{
        int rv;
        uint32_t data;

        if (power_state != POWER_STATE_S0)
                return -1;

        data = PECI_PL1_CONTROL_TIME_WINDOWS | PECI_PL1_POWER_LIMIT_ENABLE |
                PECI_PL1_POWER_LIMIT(watt);

        rv = peci_wr_pkg_config(PECI_INDEX_POWER_LIMITS_PL1, PECI_PARAMS_POWER_LIMITS_PL1,
                data);

        return rv;
}

int peci_update_PL2(int watt)
{
        int rv;
        uint32_t data;

        if (power_state != POWER_STATE_S0)
                return -1;

        data = PECI_PL2_CONTROL_TIME_WINDOWS | PECI_PL2_POWER_LIMIT_ENABLE |
                PECI_PL2_POWER_LIMIT(watt);

        rv = peci_wr_pkg_config(PECI_INDEX_POWER_LIMITS_PL2, PECI_PARAMS_POWER_LIMITS_PL2,
                data);

        return rv;
}

int peci_update_PsysPL2(int watt)
{
        int rv;
        uint32_t data;

        if (power_state != POWER_STATE_S0)
                return -1;

        data = PECI_PSYS_PL2_CONTROL_TIME_WINDOWS | PECI_PSYS_PL2_POWER_LIMIT_ENABLE |
                PECI_PSYS_PL2_POWER_LIMIT(watt);

        rv = peci_wr_pkg_config(PECI_INDEX_POWER_LIMITS_PSYS_PL2, PECI_PARAMS_POWER_LIMITS_PSYS_PL2,
                data);

        return rv;
}

int peci_update_PL4(int watt)
{
        int rv;
        uint32_t data;

        if (power_state != POWER_STATE_S0) {
            ERROR("Can't set PL4 to %d W when not in S0\n", watt);
            return -1;
        }

        data = PECI_PL4_POWER_LIMIT(watt);

        rv = peci_wr_pkg_config(PECI_INDEX_POWER_LIMITS_PL4, PECI_PARAMS_POWER_LIMITS_PL4,
                data);

        return rv;
}

// PECI information can be found here: https://www.intel.com/content/dam/www/public/us/en/documents/design-guides/core-i7-lga-2011-guide.pdf
void peci_event(void) {
    uint8_t duty;

#if EC_ESPI
    // Use PECI if CPU is not in C10 state
    if (gpio_get(&CPU_C10_GATE_N))
#else // EC_ESPI
    // Use PECI if in S0 state
    if (power_state == POWER_STATE_S0)
#endif // EC_ESPI
    {
        // Wait for completion
        while (HOSTAR & HOBY) {}
        // Clear status
        HOSTAR = HOSTAR;

        // Enable PECI, clearing data fifo's, enable AW_FCS
        HOCTLR = FIFOCLR | PECIHEN | AWFCS_EN;
        // Set address to default
        HOTRADDR = 0x30;
        // Set write length
        HOWRLR = 1;
        // Set read length
        HORDLR = 2;
        // Set command
        HOCMDR = 1;
        // Start transaction
        HOCTLR |= START;

        // Wait for completion
        while (HOSTAR & HOBY) {}

        if (HOSTAR & FINISH) {
            // Use result if finished successfully
            uint8_t low = HORDDR;
            uint8_t high = HORDDR;
            uint16_t peci_offset = ((int16_t)high << 8) | (int16_t)low;

            peci_temp = PECI_TEMP(T_JUNCTION) + peci_offset;
            duty = fan_duty(&FAN, peci_temp);
        } else {
            // Default to 50% if there is an error
            peci_temp = 0;
            duty = PWM_DUTY(50);
        }
    } else {
        // Turn fan off if not in S0 state
        peci_temp = 0;
        duty = PWM_DUTY(0);
    }

    if (fan_max) {
        // Override duty if fans are manually set to maximum
        duty = PWM_DUTY(100);
    } else {
        // Apply heatup and cooldown filters to duty
        duty = fan_heatup(&FAN, duty);
        duty = fan_cooldown(&FAN, duty);
    }

    if (duty != DCR0 || duty != DCR1) {
        DCR0 = duty;
        DCR1 = duty;
    }
}
