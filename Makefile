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
CFLAGS = -fno-builtin -nodefaultlibs -Wall -Wextra -Werror -Wno-unused-function -Wno-unused-variable
CFLAGS += -g -ffunction-sections -fdata-sections
CXXFLAGS = -std=c++2a -fno-rtti -nostdinc++ -Wno-deprecated-volatile -Wno-c99-designator
CXXFLAGS += -Wno-user-defined-literals
OBJCPYFLAGS = -O binary --strip-all
LDFLAGS = --gc-sections

TARGET ?= stm32f4
MOD_PREBUILT_DIR ?= $(BUILD_DIR)/prebuilts
RUN_ALL_TESTS ?= 0
DEBUGGABLE ?= 0

BUILD_DIR ?= ./build-$(TARGET)
SRC_DIR := ./src
SCRIPTS_DIR := ./scripts

CPPFLAGS += -I$(BUILD_DIR)/include

ifeq ($(DEBUGGABLE), 1)
CFLAGS += -Og
else
CFLAGS += -Oz -flto
endif

GLOBAL_CPPFLAGS :=
GLOBAL_CFLAGS :=
GLOBAL_CXXFLAGS :=
GLOBAL_LDFLAGS :=

c_srcs :=
asm_srcs :=
cpp_srcs :=
mod_srcs :=

include make/utils.mk
include target/$(TARGET).mk
-include make/$(ARCH).mk

RELOCABLE ?= 0

ifeq ($(RELOCABLE), 1)
CPPFLAGS += -fpie -DRELOCABLE
LDFLAGS += --pie
endif

ifeq ($(RUN_ALL_TESTS), 1)
CONFIG_INCLUDE_TESTS := y
CXXFLAGS += -DRUN_ALL_TESTS
endif

# include top directories
default_makefiles = src/Makefile
default_makefiles += src/$(BOARD_PATH)/Makefile
default_makefiles += src/arch/$(ARCH)/Makefile
default_makefiles += external/src/Makefile

$(foreach file,$(default_makefiles),$(eval $(call include_module,$(file))))

# use default linker script if none was set by target makefile
LINKER_SCRIPT ?= sc_linker.lds

CPPFLAGS += $(GLOBAL_CPPFLAGS)
CFLAGS += $(GLOBAL_CFLAGS)
CXXFLAGS += $(GLOBAL_CXXFLAGS)
CXXFLAGS += $(CFLAGS)
LDFLAGS += $(GLOBAL_LDFLAGS)

cpp_objs := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(cpp_srcs))
asm_objs := $(patsubst %.S, $(BUILD_DIR)/%.o, $(asm_srcs))
c_objs := $(patsubst %.c, $(BUILD_DIR)/%.o, $(c_srcs))
mod_pcms := $(patsubst %.cppm, $(BUILD_DIR)/%.pcm, $(mod_srcs))
mod_objs := $(patsubst %.cppm, $(BUILD_DIR)/%.o, $(mod_srcs))

objs := $(cpp_objs) $(asm_objs) $(c_objs) $(mod_objs)
deps := $(objs:.o=.d)

# this needs to be executed before any file is build
#BUILD_MOD_DEP_RESULT := $(shell mkdir -p $(MOD_PREBUILT_DIR))
#BUILD_MOD_DEP_RESULT := $(shell ./scripts/moddeps.py $(MOD_PREBUILT_DIR) $(BUILD_DIR) $(cpp_srcs) $(mod_srcs) > $(BUILD_DIR)/module-order-deps.d)

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

$(objs) : $(CONFIG_FILE) $(BUILD_DIR)/module-order-deps.d

$(CONFIG_FILE): src/$(TARGET_CONFIG_FILE)
	$(Q)mkdir -p $(dir $@)
	$(Q)cp $< $@

clean:
	$(Q)find $(BUILD_DIR) -name *.o | xargs rm -f
	$(Q)find $(BUILD_DIR) -name *.d | xargs rm -f
	$(Q)find $(BUILD_DIR) -name *.pcm | xargs rm -f
	$(Q)rm -f $(BUILD_DIR)/sc.elf $(BUILD_DIR)/sc.bin $(BUILD_DIR)/*.map $(BUILD_DIR)/_sc.lds
	$(Q)rm -f $(CONFIG_FILE)

$(BUILD_DIR)/module-order-deps.d: $(cpp_srcs) $(mod_srcs)
	$(Q)mkdir -p $(MOD_PREBUILT_DIR)
	$(Q)$(SCRIPTS_DIR)/moddeps.py $(MOD_PREBUILT_DIR) $(BUILD_DIR) $^ > $@

-include $(deps)
-include $(BUILD_DIR)/module-order-deps.d
-include src/$(BOARD_PATH)/rules.mk
