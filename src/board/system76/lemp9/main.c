#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <arch/delay.h>
#include <board/battery.h>
#include <board/gpio.h>
#include <board/gctrl.h>
#include <board/kbc.h>
#include <board/kbscan.h>
#include <board/peci.h>
#include <board/pmc.h>
#include <board/power.h>
#include <board/ps2.h>
#include <board/pwm.h>
#include <board/smbus.h>
#include <common/debug.h>
#include <common/macro.h>

uint8_t main_cycle = 0;

void external_0(void) __interrupt(0) {
    TRACE("external_0\n");
}

void timer_0(void) __interrupt(1) {
    TRACE("timer_0\n");
}

void external_1(void) __interrupt(2) {
    TRACE("external_1\n");
}

void timer_1(void) __interrupt(3) {
    TRACE("timer_1\n");
}

void serial(void) __interrupt(4) {
    TRACE("serial\n");
}

void timer_2(void) __interrupt(5) {
    TRACE("timer_2\n");
}

void init(void) {
    gpio_init();
    gctrl_init();
    kbc_init();
    pmc_init();
    kbscan_init();
    pwm_init();
    smbus_init();
    peci_init();

    //TODO: INTC
}

void ac_adapter() {
    extern struct Gpio __code ACIN_N;
    extern struct Gpio __code LED_ACIN;

    static bool send_sci = true;
    static bool last = true;

    // Check if the adapter line goes low
    bool new = gpio_get(&ACIN_N);
    // Set ACIN LED
    gpio_set(&LED_ACIN, !new);

    // If there has been a change, print
    if (new != last) {
        DEBUG("Power adapter ");
        if (new) {
            DEBUG("unplugged\n");
            battery_charger_disable();
        } else {
            DEBUG("plugged in\n");
            battery_charger_enable();
        }
        battery_debug();

        // Reset main loop cycle to force reading PECI and battery
        main_cycle = 0;

        // Send SCI to update AC and battery information
        send_sci = true;
    }

    if (send_sci) {
        // Send SCI 0x16 for AC detect event
        if (pmc_sci(&PMC_1, 0x16)) {
            send_sci = false;
        }
    }

    last = new;
}

void touchpad_event(struct Ps2 * ps2) {
    if (kbc_second) {
        *(ps2->control) = 0x07;
    } else {
        ps2_reset(ps2);
    }

    uint8_t status = *(ps2->status);
    *(ps2->status) = status;
    if (status & (1 << 3)) {
        uint8_t data = *(ps2->data);
        TRACE("touchpad: %02X\n", data);
        kbc_mouse(&KBC, data, 1000);
    }
}

void lid_event(void) {
    extern struct Gpio __code LID_SW_N;

    static bool send_sci = true;
    static bool last = true;

    // Check if the adapter line goes low
    bool new = gpio_get(&LID_SW_N);
    // If there has been a change, print
    if (new != last) {
        DEBUG("Lid ");
        if (new) {
            DEBUG("open\n");

            //TODO: send SWI if needed
        } else {
            DEBUG("closed\n");
        }

        // Send SCI
        send_sci = true;
    }

    if (send_sci) {
        // Send SCI 0x1B for lid event
        if (pmc_sci(&PMC_1, 0x1B)) {
            send_sci = false;
        }
    }

    last = new;
}

void main(void) {
    init();

    INFO("\n");

    extern struct Gpio __code SMI_N;
    extern struct Gpio __code SCI_N;
    extern struct Gpio __code SWI_N;
    extern struct Gpio __code SB_KBCRST_N;
    extern struct Gpio __code BT_EN;
    extern struct Gpio __code USB_PWR_EN_N;
    extern struct Gpio __code CCD_EN;
    extern struct Gpio __code BKL_EN;
    extern struct Gpio __code WLAN_EN;
    extern struct Gpio __code WLAN_PWR_EN;

#if GPIO_DEBUG
    gpio_debug();
#endif

    // Allow CPU to boot
    gpio_set(&SB_KBCRST_N, true);
    // Allow backlight to be turned on
    gpio_set(&BKL_EN, true);
    // Enable camera
    gpio_set(&CCD_EN, true);
    // Enable wireless
    gpio_set(&BT_EN, true);
    gpio_set(&WLAN_EN, true);
    gpio_set(&WLAN_PWR_EN, true);
    // Enable right USB port
    gpio_set(&USB_PWR_EN_N, false);
    // Assert SMI#, SCI#, and SWI#
    gpio_set(&SCI_N, true);
    gpio_set(&SMI_N, true);
    gpio_set(&SWI_N, true);

    INFO("Hello from System76 EC for %s!\n", xstr(__BOARD__));

    for(main_cycle = 0; ; main_cycle++) {
        // Enables or disables battery charging based on AC adapter
        ac_adapter();
        // Handle power states
        power_event();
        // Scans keyboard and sends keyboard packets
        kbscan_event();
        // Passes through touchpad packets
        touchpad_event(&PS2_3);
        // Handle lid close/open
        lid_event();
        // Checks for keyboard/mouse packets from host
        kbc_event(&KBC);
        // Only run the following once out of every 256 loops
        if (main_cycle == 0) {
            // Updates fan status and temps
            peci_event();
            // Updates battery status
            battery_event();
        }
        // Handles ACPI communication
        pmc_event(&PMC_1);
    }
}
