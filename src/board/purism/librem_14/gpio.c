// SPDX-License-Identifier: GPL-3.0-only

#include <board/gpio.h>
#include <common/debug.h>

struct Gpio __code ACIN_N =				GPIO(I, 5);	// ADP_IN#
struct Gpio __code AC_PRESENT =			GPIO(C, 4);	// AC_PRESENT
struct Gpio __code BKL_EN =				GPIO(E, 0);	// LCD backlight LED_ON
struct Gpio __code BT_EN =				GPIO(C, 7);	// BT_ON
struct Gpio __code BUF_PLT_RST_N =		GPIO(D, 2);
struct Gpio __code CCD_EN =				GPIO(J, 2);	// camera on/off
struct Gpio __code DD_ON =				GPIO(H, 3);	// debug VDD on/off, NA use dummy BLCTL
struct Gpio __code EC_RSMRST_N =		GPIO(B, 7);
struct Gpio __code LED_AIRPLANE =		GPIO(F, 3);	// WiFi HKS LED
struct Gpio __code LED_BAT_CHG =		GPIO(J, 5);	// green LED
struct Gpio __code LED_BAT_WARN =		GPIO(J, 4);	// orange LED
struct Gpio __code LED_PWR =			GPIO(C, 5);
struct Gpio __code LED_CAPS_LOCK =			GPIO(B, 1);
struct Gpio __code LID_SW =				GPIO(H, 6);
struct Gpio __code PCH_DPWROK_EC =		GPIO(F, 5);	// DSW_PWROK -> DPW_PWROK -> A_POWER_OK
struct Gpio __code PCH_PWROK_EC =		GPIO(G, 1);
struct Gpio __code PM_PWROK =			GPIO(E, 5);	// SYS PWR OK ???
struct Gpio __code PWR_BTN_N =			GPIO(H, 4);	// out, power button to sys
struct Gpio __code PWR_SW_N =			GPIO(E, 4);	// in, power switch state ?
struct Gpio __code SB_KBCRST_N =		GPIO(B, 6);
struct Gpio __code SCI_N =				GPIO(D, 3);	// SMC_RUNTIME_SCI#
struct Gpio __code SWI_N =				GPIO(D, 4);	// SMC_EXTSMI#
struct Gpio __code USB_PWR_EN_N =		GPIO(I, 7);	// type-C +5V enable
struct Gpio __code WLAN_EN =			GPIO(D, 5);
struct Gpio __code WLAN_PWR_EN =		GPIO(D, 0);
struct Gpio __code V095A_EN =			GPIO(F, 1);
struct Gpio __code V095A_PWRGD =		GPIO(E, 2);
struct Gpio __code V105A_EN =			GPIO(G, 6);
struct Gpio __code V105A_PWRGD =		GPIO(A, 7);
struct Gpio __code ALL_SYS_PWRGD_VRON =	GPIO(B, 5);	// ALL_SYS_PWRGD_VRON
struct Gpio __code ROP_VCCST_PWRGD =	GPIO(F, 2);	// ROP_VCCST_PWRGD
struct Gpio __code SMC_SHUTDOWN_N =		GPIO(E,1);	// power supply latch
struct Gpio __code DDR3VR_PWRGD =		GPIO(D, 1);
struct Gpio __code PM_SLP_S0_N =		GPIO(B, 0);
struct Gpio __code PM_SLP_S3_N =		GPIO(I, 4);
struct Gpio __code PM_SLP_S4_N =		GPIO(I, 3);
struct Gpio __code BAT_DETECT_N =		GPIO(I, 2);
struct Gpio __code POWER_TP_ON =		GPIO(C, 0);	// power supply for touchpad
struct Gpio __code POWER_ETH_ON =		GPIO(C, 6);	// power supply for Gbit ethernet controller
struct Gpio __code KBD_BACKLIGHT_EN =	GPIO(A, 6);
struct Gpio __code CHG_CELL_CFG =		GPIO(J, 1);
struct Gpio __code BAT_CELL_SEL =		GPIO(J, 3);
struct Gpio __code MIC_SELECT =			GPIO(F, 0);
struct Gpio __code EC_MUTE_N =			GPIO(E, 3);

void gpio_init() {
    GCR = 0x04;			// Enable LPC reset on GPD2

    // GPIO port A
    GPDRA = 0x00;		// init data with all 0 = off

    GPCRA0 = GPIO_ALT;		// fan 0 set speed
    GPCRA1 = GPIO_ALT;		// fan 1 set speed
    GPCRA2 = GPIO_ALT;		// notification LED blue
    GPCRA3 = GPIO_ALT;		// notification LED green
    GPCRA4 = GPIO_ALT;		// notification LED red
    GPCRA5 = GPIO_ALT;		// power/charging LED brightness
    GPCRA6 = GPIO_ALT;		// keyboard backlight
    GPCRA7 = GPIO_IN;		// V1.05A_PWRGD

    // GPIO port B
    GPDRB = (1 << 6) | (1 << 2);

    GPCRB0 = GPIO_IN;		// PM_SLP_S0#
    GPCRB1 = GPIO_OUT;		// capslock LED
    GPCRB2 = GPIO_OUT;		// NA (SYS_RST)
    GPCRB3 = GPIO_ALT;		// SMB_CLK_BT - battery gas gauge
    GPCRB4 = GPIO_ALT;		// SMB_DATA_BT - battery gas gauge
    GPCRB5 = GPIO_OUT;		// ALL_SYS_PWRGD_VRON
    GPCRB6 = GPIO_OUT;		// EC_PCH_RCIN#
    GPCRB7 = GPIO_OUT;		// PM_RSMRST#

    // GPIO port C
    GPDRC = (1 << 5) | (0x80);		// disable power LED

    GPCRC0 = GPIO_OUT;		// POWER_TP_ON, touchpad power on
    GPCRC1 = GPIO_ALT;		// I2C_CLK1, NA
    GPCRC2 = GPIO_ALT;		// I2C_DATA1, NA
    GPCRC3 = GPIO_IN;		// PM_SLP_SUS#, NA
    GPCRC4 = GPIO_OUT;		// AC_PRESENT
    GPCRC5 = GPIO_OUT;		// pwr/charging white LED enable
    GPCRC6 = GPIO_OUT;		// GBit LAN port power supply enable/disable
    GPCRC7 = GPIO_OUT;		// W_DISABLE2 on WiFi/BT M.2 slot

    // GPIO port D
    //GPDRD = (1 << 4) | (1 << 3) | (0x80);
    GPDRD = (uint8_t) 0x98U;

    GPCRD0 = GPIO_OUT;		// WiFi/BT M.2 power supply on/off
    GPCRD1 = GPIO_IN;		// DDR3VR_PWRGD
    GPCRD2 = GPIO_ALT;		// PLT_RST_BUF#, LPC reset
    GPCRD3 = GPIO_OUT;		// SMC_RUNTIME_SCI#
    GPCRD4 = GPIO_OUT;		// SMC_EXTSMI#
    GPCRD5 = GPIO_OUT;		// W_DISABLE1 on WiFi/BT M.2 slot
    GPCRD6 = GPIO_ALT;		// fan 0 tacho input
    GPCRD7 = GPIO_ALT;		// fan 1 tacho input

    // GPIO port E
    GPDRE = (1 << 0) | (1 << 1) | (1 << 3) | 0x80 /*(1U << 7)*/;	// disable BAT_LOW

    GPCRE0 = GPIO_OUT;		// LCD LED backlight enable
    GPCRE1 = GPIO_OUT;		// system power off, SMC_SHUTDOWN#
    GPCRE2 = GPIO_IN;		// V0.95A_PWRGD
    GPCRE3 = GPIO_OUT;		// audio codec power down, EC_MUTE#
    GPCRE4 = GPIO_IN;		// power status feedback, systems powered on
    GPCRE5 = GPIO_OUT;		// ROP_SYS_PWROK, SYS_PWROK on main CPU
    GPCRE6 = GPIO_IN;		// PM_SUS_STAT#
    GPCRE7 = GPIO_OUT;		// PM_BATLOW#

    // GPIO port F
    GPDRF = (1 << 5);

    GPOTF = 1;  // Open drain on MIC_SPDIF_SEL

    GPCRF0 = GPIO_OUT;		// audio codec SPDIF0/GPIO2 -> toggle for headphone mic input
    GPCRF1 = GPIO_OUT;		// +V0.95A_EN
    GPCRF2 = GPIO_OUT;		// ROP_VCCST_PWRGD
    GPCRF3 = GPIO_OUT;		// WiFi/BT HKS white LED enable
    GPCRF4 = GPIO_OUT;		// ME_FLASH
    GPCRF5 = GPIO_OUT;		// A_POWER_OK -> DSW_PWROK
    GPCRF6 = GPIO_ALT;		// H_PECI_EC
    GPCRF7 = GPIO_IN;		// NA

    // GPIO port G
    GPDRG = 0x00;

    GPCRG1 = GPIO_IN;		// PCH_PWROK_EC
    GPCRG6 = GPIO_OUT;		// V1.05A_EN

    // GPIO port H
    GPDRH = (1 << 4) | (1 << 3);

    GPCRH0 = GPIO_ALT;		// LPC_CLKRUN#
    GPCRH1 = GPIO_ALT;		// I2C CLK, BQ24715 battery charge controller
    GPCRH2 = GPIO_ALT;		// I2C DATA, BQ24715 battery charge controller
    GPCRH3 = GPIO_OUT;		// BLCTL, LCD LED backlight control, NA
    GPCRH4 = GPIO_OUT;		// PM_EC_PWRBTN#
    GPCRH5 = GPIO_IN;		// TYPE_EC_IRQ -> type-C ALERT#
    GPCRH6 = GPIO_IN;		// LCD lid open/close switch
    GPCRH7 = GPIO_IN;		// NA

    // GPIO port I
    GPDRI = (1U << 7);

    GPCRI0 = GPIO_ALT;		// Chager_IOUT, BQ24715 IOUT, analog charge/discharge current
    GPCRI1 = GPIO_ALT;		// +VBAT analog input
    GPCRI2 = GPIO_IN;		// battery pack detect
    GPCRI3 = GPIO_IN;		// PM_SLP_S4#
    GPCRI4 = GPIO_IN;		// PM_SLP_S3#
    GPCRI5 = GPIO_IN;		// ADP_IN#, DC adapter (charger) present
    GPCRI6 = GPIO_ALT;		// 3.5mm jack headphone detect, analog sense
    GPCRI7 = GPIO_OUT;		// type-C USB port 5V power enable

    // GPIO port J
    GPDRJ = (1 << 3) | (1 << 4) | (1 << 5); // LEDs have negative logic, turn off

    GPCRJ0 = GPIO_OUT;		// PROCHOT#_EC, device overheat
    GPCRJ1 = GPIO_OUT;		// DIS_BAT, CELL config for charge controller
    GPCRJ2 = GPIO_OUT;		// camera power on
    GPCRJ3 = GPIO_OUT;		// BAT_CELL_SEL
    GPCRJ4 = GPIO_OUT;		// power/charging LED green enable/disable
    GPCRJ5 = GPIO_OUT;		// power/charging LED orange enable/disable

    // GPIO port M
    GPDRM = 0x00;

    GPCRM0 = GPIO_ALT;		// LPC_AD0
    GPCRM1 = GPIO_ALT;		// LPC_AD1
    GPCRM2 = GPIO_ALT;		// LPC_AD2
    GPCRM3 = GPIO_ALT;		// LPC_AD3
    GPCRM4 = GPIO_ALT;		// EC_LPC_CLK
    GPCRM5 = GPIO_ALT;		// LPC_FRAME#
    GPCRM6 = GPIO_ALT;		// LPC_SERIRQ

}

#if GPIO_DEBUG
void gpio_debug_bank(
    char * bank,
    uint8_t data,
    uint8_t mirror,
    uint8_t pot,
    volatile uint8_t * control
) {
    for(char i = 0; i < 8; i++) {
        DEBUG(
            "%s%d:\n\tdata %d\n\tmirror %d\n\tpot %d\n\tcontrol %02X\n",
            bank,
            i,
            (data >> i) & 1,
            (mirror >> i) & 1,
            (pot >> i) & 1,
            *(control + i)
        );
    }
}

void gpio_debug(void) {
    #define bank(BANK) gpio_debug_bank(#BANK, GPDR ## BANK, GPDMR ## BANK, GPOT ## BANK, &GPCR ## BANK ## 0)
    bank(A);
    bank(B);
    bank(C);
    bank(D);
    bank(E);
    bank(F);
    bank(G);
    bank(H);
    bank(I);
    bank(J);
    #undef bank
}
#endif
