// SPDX-License-Identifier: GPL-3.0-only

#include <board/jack_detect.h>
#include <board/gpio.h>
#include <board/power.h>
#include <common/debug.h>
#include <arch/delay.h>

#if defined(HAVE_JACK_DETECT)

// Persistent state for headphone jack measurements.  (As usual, EC has very
// little RAM, so we pack this as tightly as possible.)
struct
{
    // The last jack detect value sensed.  Cleared when the jack detect is
    // disabled.
    bool last_jack_det:1;
    // Whether the jack detect is enabled; disabled in any sleep state.
    bool enabled:1;
} jack_detect_state = {0};

static void set_jack_detect(bool state)
{
    if (jack_detect_state.last_jack_det == state)
        return; // Nothing to do

    jack_detect_state.last_jack_det = state;
    if (jack_detect_state.last_jack_det) {
        DEBUG("Jack detect: plug inserted\n");
        // Tri-state, plug present
        *(MIC_SELECT.data) |= MIC_SELECT.value;
        // Though there are pull-ups on this net, we have to briefly push high
        // for the codec to sense the change.  The mic select is multiplexed
        // with the second digital microphone's data line (DMIC_DATA1), and this
        // seems to cause the codec to switch the pad mux.
        GPOTF &= ~1;    // push-pull, go high briefly
        delay_ms(1);
        // We don't want to stay pushing high forever.  If the S3 plane powers
        // off while pushing high (which can happen suddenly), we'll be
        // backfeeding another rail and that rail will go into a safety shutoff
        // until its own power is removed (disconnect AC adapter for 10
        // seconds).  The codec will stay in this state until we actively pull
        // low again.
        GPOTF |= 1;     // back to open drain
    } else {
        DEBUG("Jack detect: plug removed\n");
        // Pull low, no plug
        *(MIC_SELECT.data) &= ~MIC_SELECT.value;
    }
}

static void enable_jack_detect(bool enable)
{
    if (enable == jack_detect_state.enabled)
        return; // Nothing to do

    jack_detect_state.enabled = enable;
    if (enable) {
        DEBUG("Jack detect is enabled\n");
        board_jack_detect_activate();
    } else {
        DEBUG("Jack detect is disabled\n");
        set_jack_detect(false);
    }
}

void jack_detect_event()
{
    enable_jack_detect(power_state == POWER_STATE_S0);
}

void jack_detect_1s_event()
{
    if (!jack_detect_state.enabled)
        return; // Not enabled, nothing to do

    bool new_jack_det = board_jack_detect_sense(jack_detect_state.last_jack_det);
    set_jack_detect(new_jack_det);
}

#endif // defined(HAVE_JACK_DETECT)
