
BOARD_PATH := board/raspberrypi/rpip
TARGET_CONFIG_FILE := board/raspberrypi/rpip/config.h
LINKER_SCRIPT := src/soc/rp2040/linker.ld
CONFIG_PL011 := y
CONFIG_LIBAEABI_CORTEX_M0 := y
CONFIG_NVIC := y
CONFIG_SOC_RP2040 := y
CONFIG_LIB_GPIO := y
CONFIG_LIB_I2C := y
CONFIG_LIB_SPI := y

CPU := armv6m

ARCH = arm

GLOBAL_CPPFLAGS += -DARMV6M -target armv6m-none-eabi -mcpu=cortex-m0plus
GLOBAL_CFLAGS += -mthumb -mfloat-abi=soft
