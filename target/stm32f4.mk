
BOARD_PATH := board/st/stm32f4/
TARGET_CONFIG_FILE := board/st/stm32f4/config.h
LINKER_SCRIPT := src/board/st/stm32f4/linker.lds
CONFIG_PL011 := y

CPU := armv7m

ARCH = arm

GLOBAL_CFLAGS += -DARMV7M -DARM -target armv7m-none-eabihf -mcpu=cortex-m4 -mthumb
