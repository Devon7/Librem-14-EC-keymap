// SPDX-License-Identifier: GPL-3.0-only

#ifndef _BOARD_JACK_DETECT_H
#define _BOARD_JACK_DETECT_H

#include <stdbool.h>

#if defined(HAVE_JACK_DETECT)
// Jack detect fast task.  Disables the jack detect if not in S0.  We do this in
// the fast task because it is important that we never try to assert the jack
// detect if we return to S0 but the codec hasn't been configured yet.
void jack_detect_event();
// Jack detect 1 second task.  If in S0 and jack detect is enabled, checks
// board's jack sense and updates jack detect GPIO.
void jack_detect_1s_event();

// Boards with HAVE_JACK_DETECT must provide the following:

// board_jack_detect_sense() senses whether anything is currently plugged into
// the headphone jack.  This is called every 1 second when the jack detect is
// active.  If no new sense measurement is ready, it may return the last
// sensed value (such as if an ADC measurement is not complete yet).
bool board_jack_detect_sense(bool last_sense);

// board_jack_detect_activate() is called when the jack detect becomes active;
// we are in S0 _and_ the jack detect is enabled.  The board can activate ADC
// measurements, etc.
void board_jack_detect_activate(void);

#endif // defined(HAVE_JACK_DETECT)

#endif // _BOARD_SMFI_H
