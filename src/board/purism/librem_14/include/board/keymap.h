// SPDX-License-Identifier: GPL-3.0-only

#ifndef _BOARD_KEYMAP_H
#define _BOARD_KEYMAP_H

// Keymap layers (normal, Fn)
#define KM_LAY 2
// Keymap output pins
#define KM_OUT 16
// Keymap input pins
#define KM_IN 8

// common/keymap.h requires KM_LAY, KM_OUT, and KM_IN definitions
#include <common/keymap.h>

// International keys
#ifndef KI1
    #define KI1 K_INT_1
#endif
#ifndef KI2
    #define KI2 K_INT_2
#endif

// Conversion of physical layout to keyboard matrix
#define LAYOUT( \
    K110, K112, K113, K114, K115, K116, K117, K118, K119, K120, K121, K122, K123, K124, K76, \
    K1, K2, K3, K4, K5, K6, K7, K8, K9, K10, K11, K12, K13, K15, \
    K16, K17, K18, K19, K20, K21, K22, K23, K24, K25, K26, K27, K28, K29, \
    K30, K31, K32, K33, K34, K35, K36, K37, K38, K39, K40, K41, K43, \
    K44, K46, K47, K48, K49, K50, K51, K52, K53, K54, K55, K83, K57, \
    K58, K59, K127, K60, K61, K62, K64, K79, K84, K89 \
) { \
    { ___, K89, ___, K124, K76, ___, ___, ___ }, \
    { K41, K43, ___, ___, K13, K15, K28, K29 }, \
    { K55, K83, K12, K123, K11, K26, K27, K40 }, \
    { ___, K58, ___, ___, K64, ___, ___, ___ }, \
    { ___, ___, K119, K120, ___, ___, ___, K79 }, \
    { K53, ___, K118, K117, K116, K9, K24,K38 }, \
    { K57, K44, ___, ___, ___, ___, ___, ___ }, \
    { K54, K84, ___, K122, K121, K10, K25, K39 }, \
    { K52, K51, K36, K22, K7, K8, K23, K37 }, \
    { K49, K50, K35, K21, K6, K5, K20, K34 }, \
    { K47, K46, K113, K112, K110, K3, K18, K32 }, \
    { K48, K61, K114, K115, K30, K4, K19, K33 }, \
    { K62, K60, ___, ___, ___, ___, ___, ___ }, \
    { ___, ___, K59, K16, K1, K2, K17, K31 }, \
    { ___, K127, ___, ___, ___, ___, ___, ___ }, \
    { ___, ___, ___, ___, ___, ___, ___, ___ } \
}

// Position of physical Esc key in the matrix
// ../../keymap/devon.c K_ESC = K110 = column 4 row 10
// ../../keymap/default.c K_ESC = K110 = column 4 row 10
#define MATRIX_ESC_INPUT    4
#define MATRIX_ESC_OUTPUT   10

// Position of physical Fn key in the matrix
#ifdef DEVON
// ../../keymap/devon.c KT_FN = LCTRL_FN = K58 = column 1 row 3
#define MATRIX_FN_INPUT     1
#define MATRIX_FN_OUTPUT    3
#else // DEVON
// ../../keymap/devon.c KT_FN = FN_LCTRL = K59 = column 2 row 13
// ../../keymap/default.c KT_FN = K59 = column 2 row 13
#define MATRIX_FN_INPUT     2
#define MATRIX_FN_OUTPUT    13
#endif // DEVON

#endif // _BOARD_KEYMAP_H
