// SPDX-License-Identifier: GPL-3.0-only
// librem-ec/src/board/purism/librem_14/keymap/devon.c

// Devonian layout
// Swap CapsLock & BackSpace here, not in kmonad --
//	kmonad-mapped CapsLock autorepeat fails in Mozilla.
// Kmonad best customizes the key left of (tiny) RightShift --
//	with tap/hold dual functions on the same key.
// Swap LeftControl & Fn -- like on ThinkPads and MacBooks
//	where the Fn key is in the lower left corner.
// TO DO
// Dynamic keymapping should support all such customizations --
//	without breaking the Fn+Esc reset function.
// Fn+F1 & Fn+F2 should do something useful.
// Should Fn+Space duplicate Fn+F4 cycle keyboard backlight glow?
// Solve FireFox & Thunderbird physical CapsLock autorepeat failure.

#include <board/keymap.h>
#ifdef DEVON
  #define BKSP_CAPS K_CAPS
  #define CAPS_BKSP K_BKSP
  #define LCTRL_FN KT_FN
  #define FN_LCTRL K_LEFT_CTRL
#else // DEVON
  #define BKSP_CAPS K_BKSP
  #define CAPS_BKSP K_CAPS
  #define LCTRL_FN K_LEFT_CTRL
  #define FN_LCTRL KT_FN
#endif // DEVON

uint16_t __code KEYMAP[KM_LAY][KM_OUT][KM_IN] = {
// regular key layout
LAYOUT(
    K_ESC, K_F1, K_F2, K_F3, K_F4, K_F5, K_F6, K_F7, K_F8, K_F9, K_F10, K_F11, K_F12, K_PRINT_SCREEN, K_DEL,
    K_TICK, K_1, K_2, K_3, K_4, K_5, K_6, K_7, K_8, K_9, K_0, K_MINUS, K_EQUALS, BKSP_CAPS,
    K_TAB, K_Q, K_W, K_E, K_R, K_T, K_Y, K_U, K_I, K_O, K_P, K_BRACE_OPEN, K_BRACE_CLOSE, K_BACKSLASH,
    CAPS_BKSP, K_A, K_S, K_D, K_F, K_G, K_H, K_J, K_K, K_L, K_SEMICOLON, K_QUOTE, K_ENTER,
    K_LEFT_SHIFT, K_Z, K_X, K_C, K_V, K_B, K_N, K_M, K_COMMA, K_PERIOD, K_SLASH, K_UP, K_RIGHT_SHIFT,
    LCTRL_FN, FN_LCTRL, K_LEFT_SUPER, K_LEFT_ALT, K_SPACE, K_RIGHT_ALT, K_RIGHT_CTRL, K_LEFT, K_DOWN, K_RIGHT
),
// FN+key layout
LAYOUT(
    K_SUSPEND, K_F1, K_F2, K_DISPLAY_MODE, K_KBD_BKL, K_BRIGHTNESS_DOWN, K_BRIGHTNESS_UP, K_MEDIA_PREV, K_PLAY_PAUSE, K_MEDIA_NEXT, K_MUTE, K_VOLUME_DOWN, K_VOLUME_UP, K_PRINT_SCREEN, K_INSERT,
    K_TICK, K_1, K_2, K_3, K_4, K_5, K_6, K_7, K_8, K_9, K_0, K_MINUS, K_EQUALS, BKSP_CAPS,
    K_TAB, K_Q, K_W, K_E, K_R, K_T, K_Y, K_U, K_I, K_O, K_P, K_BRACE_OPEN, K_BRACE_CLOSE, K_BACKSLASH,
    CAPS_BKSP, K_A, K_S, K_D, K_F, K_G, K_H, K_J, K_K, K_L, K_SEMICOLON, K_QUOTE, K_ENTER,
    K_LEFT_SHIFT, K_Z, K_X, K_C, K_V, K_B, K_N, K_M, K_COMMA, K_PERIOD, K_SLASH, K_PGUP, K_RIGHT_SHIFT,
    LCTRL_FN, FN_LCTRL, K_LEFT_SUPER, K_LEFT_ALT, K_KBD_BKL, K_RIGHT_ALT, K_RIGHT_CTRL, K_HOME, K_PGDN, K_END
)
};

// end librem-ec/src/board/purism/librem_14/keymap/devon.c