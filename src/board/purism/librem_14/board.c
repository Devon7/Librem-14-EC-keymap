// SPDX-License-Identifier: GPL-3.0-only

#include <board/battery.h>
#include <board/board.h>
#include <board/gpio.h>
#include <board/power.h>
#include <board/kbc.h>
#include <ec/pwm.h>
#include <ec/adc.h>
#include <ec/bram.h>
#include <common/debug.h>

extern uint8_t main_cycle;


// persistent settings stored in BRAM bank#1 @ 0x80..0xbf
#define BRAM_OFFSET 0x80
#define BRAM_CHARGE_START_THRES		(BRAM_OFFSET + 0)
#define BRAM_CHARGE_END_THRES		(BRAM_OFFSET + 1)

// see if BRAM has valid content, if not clear it and mark
bool bram_init(void) {
    int i;
    uint8_t sum=42;

    for (i=0x80; i<(0xbf); i++)
        sum ^= BRAM[i];
    if (sum == BRAM[0xbf]) {
        DEBUG("BRAM valid\n");
        return true;
    } else {
        DEBUG("BRAM invalid, clearing\n");
        for (i=0x80; i<(0xbf); i++)
            BRAM[i] = 0x00;
        BRAM[0xbf] = 42;
        return false;
    }
}

bool bram_set_value(uint8_t offset, uint8_t value)
{
    int i;
    uint8_t sum=42;

    if (offset < 0x80)
        return false;

    BRAM[offset] = value;
    for (i=0x80; i<(0xbf); i++)
        sum ^= BRAM[i];
    BRAM[0xbf] = sum;

    return true;
}

void board_init(void) {
    // Enable camera
    gpio_set(&CCD_EN, true);
    // Enable wireless
    gpio_set(&BT_EN, true);
    gpio_set(&WLAN_EN, true);

    // Allow CPU to boot
    gpio_set(&SB_KBCRST_N, true);

    // Allow backlight to be turned on
    gpio_set(&BKL_EN, true);

    // Assert SCI#, and SWI#
    gpio_set(&SCI_N, true);
    gpio_set(&SWI_N, true);

    // in case we got powered up running from battery only
    // we need to make sure power keeps up
    // gpio_set(&SMC_SHUTDOWN_N, true);

    adc_init();
    VCH0CTL = (1 << 0);	// VCH0 = ADC input 1 on GPI1, clear all else, bat voltage
    VCH1CTL = 0x00;	// VCH1 = ADC input 0 on GPI0, clear all else, charge/discharge currrent
    adc_enable(true); // we need ADC channel 1 to read bat voltage

    // I2C3 enable
    GCR2 |= (1 << 5); // SMB3E

    // PWMs
    // Turn off fans, get controlled by PECI
    DCR0 = 0x00;
    DCR1 = 0x00;

    // turn off notification LED RGB
    DCR2 = 0x00; // B
    DCR3 = 0x00; // G
    DCR4 = 0x00; // R

    // enable power LED control full brightness
    DCR5 = 0xff;

    // keyboard backlight PWM, zero at init
    DCR6 = 0x00;

    gpio_set(&LED_BAT_CHG, true);
    gpio_set(&LED_BAT_WARN, true);
    battery_charger_disable();
    board_battery_init();

    if (bram_init()) {
        DEBUG("BRAM OK, start %d end %d\n", BRAM[BRAM_CHARGE_START_THRES], BRAM[BRAM_CHARGE_END_THRES]);
        battery_set_start_threshold(BRAM[BRAM_CHARGE_START_THRES]);
        battery_set_end_threshold(BRAM[BRAM_CHARGE_END_THRES]);
    } else
        battery_reset();
}

void board_on_ac(bool ac) {
    DEBUG("board ac %s\n", ac ? "t" : "f");
}

// called every main loop cycle, careful
void board_event(void) {
    static uint8_t last_kbc_leds = 0;

    if (main_cycle == 0) {
        // Set keyboard LEDs
        if (kbc_leds != last_kbc_leds) {
            gpio_set(&LED_CAPS_LOCK, (kbc_leds & 4));
            last_kbc_leds = kbc_leds;
        }
    }
}

// check headphone jack plug state, set GPIO direction
static void check_jack_state(void) {
    static bool hp_det=false;

    if (power_state == POWER_STATE_S0) {
        bool hp_det_curr = !gpio_get(&HEADPHONE_DET);
        if (hp_det != hp_det_curr) {
            hp_det = hp_det_curr;
            if (hp_det) {
                DEBUG("Jack detect: plug inserted\n");
                // there is a pull up so setting it as input will pull it high too
                *(MIC_SELECT.control) = GPIO_IN;
            } else {
                DEBUG("Jack detect: plug removed\n");
                // pin state is false by GPIO init, so just reenable output to pull it low
                *(MIC_SELECT.control) = GPIO_OUT;
            }
        }
    } else {
        // default to unplugged when not in S0
        *(MIC_SELECT.control) = GPIO_OUT;
    }
}

// called once per second
void board_1s_event(void) {
    check_jack_state();

    // if battery voltage drops below min voltage we prepare to shut down
    // hard once external power is removed to prevent main battery from
    // deep discharge
    if (battery_status && BATTERY_INITIALIZED) {
        if ((battery_voltage <= battery_min_voltage) /*||
            (battery_status & BATTERY_FULLY_DISCHARGED)*/) {
            gpio_set(&SMC_SHUTDOWN_N, false);
        }
    }
}
