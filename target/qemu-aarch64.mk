

BOARD_PATH := board/qemu/aarch64
TARGET_CONFIG_FILE := board/qemu/aarch64/config.h
CONFIG_PL011 := y

include make/cpu/armv8.mk
