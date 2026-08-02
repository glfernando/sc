

BOARD_PATH := board/raspberrypi/rpi4
TARGET_CONFIG_FILE := board/raspberrypi/rpi4/config.h
CONFIG_PL011 := y
CONFIG_CPU_SOC_DUMMY := y

include make/cpu/armv8.mk
