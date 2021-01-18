# SPDX-License-Identifier: BSD-3-Clause
#
# (C) Copyright 2020, Fernando Lugo <lugo.fernando@gmail.com>

CC = clang++
LD = ld.lld
OBJCOPY = llvm-objcopy

# Quiet mode
ifeq ($(V), 1)
Q =
else
Q = @
endif

CPPFLAGS = -Isrc/include -include config.h -MMD -MP
CFLAGS = -fno-builtin -nodefaultlibs -fpie -Oz -Wall -Wextra -Werror -flto
CFLAGS += -g -ffunction-sections -fdata-sections
CXXFLAGS = -std=c++2a -fno-rtti -nostdinc++ -Wno-deprecated-volatile
CXXFLAGS += -Wno-user-defined-literals
OBJCPYFLAGS = -O binary --strip-all
LDFLAGS = --gc-sections --pie

TARGET ?= qemu-aarch64
MOD_PREBUILT_DIR ?= $(BUILD_DIR)/prebuilts
RUN_ALL_TESTS ?= 0

BUILD_DIR ?= ./build-$(TARGET)
SRC_DIR := ./src

CPPFLAGS += -I$(BUILD_DIR)/include

srcs :=
mod_srcs :=
dirs :=
GLOBAL_CPPFLAGS :=
GLOBAL_CFLAGS :=
GLOBAL_CXXFLAGS :=
GLOBAL_LDFLAGS :=

include make/utils.mk
include target/$(TARGET).mk
-include make/$(ARCH).mk

ifeq ($(RUN_ALL_TESTS), 1)
CONFIG_INCLUDE_TESTS := y
CXXFLAGS += -DRUN_ALL_TESTS
endif

# include top directories
include src/Makefile
include src/$(BOARD_PATH)/Makefile
include src/arch/$(ARCH)/Makefile
include external/src/Makefile

# use default linker script if none was set by target makefile
LINKER_SCRIPT ?= sc_linker.lds

CPPFLAGS += $(GLOBAL_CPPFLAGS)
CFLAGS += $(GLOBAL_CFLAGS)
CXXFLAGS += $(GLOBAL_CXXFLAGS)
CXXFLAGS += $(CFLAGS)
LDFLAGS += $(GLOBAL_LDFLAGS)

cpp_srcs := $(filter %.cpp, $(srcs))
asm_srcs := $(filter %.S, $(srcs))
c_srcs := $(filter %.c, $(srcs))
cpp_objs := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(cpp_srcs))
asm_objs := $(patsubst %.S, $(BUILD_DIR)/%.o, $(asm_srcs))
c_objs := $(patsubst %.c, $(BUILD_DIR)/%.o, $(c_srcs))
mod_pcms := $(patsubst %.cppm, $(BUILD_DIR)/%.pcm, $(mod_srcs))
mod_objs := $(patsubst %.cppm, $(BUILD_DIR)/%.o, $(mod_srcs))

objs := $(cpp_objs) $(asm_objs) $(c_objs) $(mod_objs)
deps := $(objs:.o=.d)

# this needs to be executed before any file is build
BUILD_MOD_DEP_RESULT := $(shell mkdir -p $(MOD_PREBUILT_DIR))
BUILD_MOD_DEP_RESULT := $(shell ./scripts/moddeps.py $(MOD_PREBUILT_DIR) . $(BUILD_DIR) > $(BUILD_DIR)/module-order-deps.d)

CONFIG_FILE = $(BUILD_DIR)/include/config.h

.PHONY: clean

all: $(BUILD_DIR)/sc.bin

$(BUILD_DIR)/sc.bin: $(BUILD_DIR)/sc.elf
	$(Q)$(OBJCOPY) $(OBJCPYFLAGS) $< $@

$(BUILD_DIR)/sc.elf : $(objs) $(BUILD_DIR)/_sc.lds
	$(Q)$(LD) $(LDFLAGS) -T $(BUILD_DIR)/_sc.lds -o $@ $(objs)

$(BUILD_DIR)/_sc.lds : $(LINKER_SCRIPT)
	$(Q)$(CC) $(CPPFLAGS) -E -x assembler-with-cpp -P -o $@ $<

$(BUILD_DIR)/%.o : %.S
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o : %.c
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -x c -c $< -o $@

$(BUILD_DIR)/%.o : %.cpp
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) $(CPPFLAGS) $(CXXFLAGS) -fimplicit-modules -fimplicit-module-maps -fprebuilt-module-path=$(MOD_PREBUILT_DIR)/ -c $< -o $@

$(BUILD_DIR)/%.pcm : %.cppm
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) $(CPPFLAGS) $(CXXFLAGS) -fimplicit-modules -fimplicit-module-maps -fmodules -fprebuilt-module-path=$(MOD_PREBUILT_DIR)/ --precompile $< -o $@

$(BUILD_DIR)/%.o : $(BUILD_DIR)/%.pcm
	$(Q)$(CC) $(CPPFLAGS) $(CXXFLAGS) -fimplicit-modules -fimplicit-module-maps -fprebuilt-module-path=$(MOD_PREBUILT_DIR)/ -Wno-unused-command-line-argument -c $< -o $@

$(objs) : $(CONFIG_FILE)

$(CONFIG_FILE): src/$(TARGET_CONFIG_FILE)
	$(Q)mkdir -p $(dir $@)
	$(Q)cp $< $@

clean:
	$(Q)find $(BUILD_DIR) -name *.o | xargs rm -f
	$(Q)find $(BUILD_DIR) -name *.d | xargs rm -f
	$(Q)find $(BUILD_DIR) -name *.pcm | xargs rm -f
	$(Q)rm -f $(BUILD_DIR)/sc.elf $(BUILD_DIR)/sc.bin $(BUILD_DIR)/*.map $(BUILD_DIR)/_sc.lds
	$(Q)rm -f $(CONFIG_FILE)

-include $(deps)
-include $(BUILD_DIR)/module-order-deps.d
-include src/$(BOARD_PATH)/rules.mk
