
BOARD_PATH := board/raspberrypi/rpip
TARGET_CONFIG_FILE := board/raspberrypi/rpip/config.h
LINKER_SCRIPT := src/board/raspberrypi/rpip/linker.ld
CONFIG_PL011 := y
CONFIG_LIBAEABI_CORTEX_M0 := y
CONFIG_NVIC := y
CONFIG_SOC_RP2040 := y

CPU := armv6m

ARCH = arm

GLOBAL_CFLAGS += -DARMV6M -target armv6m-none-eabi -mcpu=cortex-m0plus -mthumb -mfloat-abi=soft
