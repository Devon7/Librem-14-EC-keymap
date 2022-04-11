# SPDX-License-Identifier: GPL-3.0-only

EC=it8528e

# Add keymap to src
KEYMAP?=default
SRC+=$(BOARD_DIR)/keymap/$(KEYMAP).c

# Set keyboard LED mechanism
KBLED=white_pwm

# Set charger I2C bus
CFLAGS+=-DI2C_SMBUS=I2C_3

# Set battery I2C bus
CFLAGS+=-DI2C_BATTERY=I2C_0

# Set I2C bus for debug output
CFLAGS+=-DI2C_DEBUG_BUS=I2C_0

CFLAGS+=-DHAVE_LID_SW

# Custom fan curve
CFLAGS+=-DBOARD_HEATUP=5
CFLAGS+=-DBOARD_COOLDOWN=20
CFLAGS+=-DBOARD_FAN_POINTS="\
	FAN_POINT(50, 20), \
	FAN_POINT(60, 25), \
	FAN_POINT(65, 30), \
	FAN_POINT(70, 40), \
	FAN_POINT(75, 60), \
	FAN_POINT(80, 75), \
	FAN_POINT(85, 90), \
	FAN_POINT(90, 100) \
"
# Set CPU power limits in watts
#CFLAGS+=\
#	-DPOWER_LIMIT_AC=45 \
#	-DPOWER_LIMIT_DC=20

# Add purism common code
include src/board/purism/common/common.mk
