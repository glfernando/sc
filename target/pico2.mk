BOARD_PATH := board/raspberrypi/pico2
TARGET_CONFIG_FILE := board/raspberrypi/pico2/config.h
LINKER_SCRIPT := src/soc/rp2350/linker.ld
CONFIG_PL011 := y
CONFIG_LIBAEABI_CORTEX_M0 := y
CONFIG_NVIC := y
CONFIG_SOC_RP2350 := y
CONFIG_INCLUDE_TESTS := y

CPU := armv8m
ARCH = arm

GLOBAL_CPPFLAGS += -DARMV8M -target thumbv8m.main-none-eabihf -mcpu=cortex-m33
GLOBAL_CFLAGS += -mthumb -mfloat-abi=hard
