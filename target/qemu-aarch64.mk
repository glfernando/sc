

BOARD_PATH := board/qemu/aarch64
TARGET_CONFIG_FILE := board/qemu/aarch64/config.h
CONFIG_PL011 := y
CONFIG_CPU_SOC_DUMMY := y
CONFIG_INCLUDE_TESTS := y

include make/cpu/armv8.mk
