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

CPPFLAGS = -Isrc/include -include config.h
CFLAGS = -fno-builtin -nodefaultlibs -fpie -Oz -Wall -Wextra -Werror -flto
CFLAGS += -g -ffunction-sections -fdata-sections
CXXFLAGS = -std=c++2a -fno-rtti -nostdinc++ -Wno-deprecated-volatile
CXXFLAGS += -Wno-user-defined-literals
OBJCPYFLAGS = -O binary --strip-all
LDFLAGS = --gc-sections --pie

TARGET ?= qemu-aarch64
MOD_PREBUILT_DIR ?= $(BUILD_DIR)/prebuilts

BUILD_DIR ?= ./build-$(TARGET)
SRC_DIR := ./src

CPPFLAGS += -I$(BUILD_DIR)/include

srcs :=
mod_srcs :=
dirs :=
GLOBAL_CPPFLAGS :=
GLOBAL_CFLAGS :=
GLOBAL_CXXFLAGS :=

include make/utils.mk
include target/$(TARGET).mk
-include make/$(ARCH).mk

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

cpp_srcs := $(filter %.cpp, $(srcs))
asm_srcs := $(filter %.S, $(srcs))
c_srcs := $(filter %.c, $(srcs))
cpp_objs := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(cpp_srcs))
asm_objs := $(patsubst %.S, $(BUILD_DIR)/%.o, $(asm_srcs))
c_objs := $(patsubst %.c, $(BUILD_DIR)/%.o, $(c_srcs))
mod_pcms := $(patsubst %.cppm, $(BUILD_DIR)/%.pcm, $(mod_srcs))
mod_objs := $(patsubst %.cppm, $(BUILD_DIR)/%.o, $(mod_srcs))

objs := $(cpp_objs) $(asm_objs) $(c_objs) $(mod_objs)

.PHONY: clean modules config_file

all: $(BUILD_DIR)/sc.bin

$(BUILD_DIR)/sc.bin: $(BUILD_DIR)/sc.elf | modules
	$(Q)$(OBJCOPY) $(OBJCPYFLAGS) $< $@

$(BUILD_DIR)/sc.elf : $(objs) | $(BUILD_DIR)/_sc.lds
	$(Q)$(LD) $(LDFLAGS) -T $(BUILD_DIR)/_sc.lds -o $@ $^

$(BUILD_DIR)/_sc.lds : $(LINKER_SCRIPT)
	$(Q)$(CC) $(CPPFLAGS) -E -x assembler-with-cpp -P -o $@ $<

$(BUILD_DIR)/%.o : %.S | config_file
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o : %.c | config_file
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -x c -c $< -o $@

$(BUILD_DIR)/%.o : %.cpp | modules config_file
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) $(CPPFLAGS) $(CXXFLAGS) -fimplicit-modules -fimplicit-module-maps -fprebuilt-module-path=$(MOD_PREBUILT_DIR)/ -c $< -o $@

modules: $(mod_pcms)

$(BUILD_DIR)/%.pcm : %.cppm | config_file
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) $(CPPFLAGS) $(CXXFLAGS) -fimplicit-modules -fimplicit-module-maps -fmodules -fprebuilt-module-path=$(MOD_PREBUILT_DIR)/ --precompile $< -o $@
	$(Q)cp $@ $(MOD_PREBUILT_DIR)/$(shell grep "^export module" $< | awk '{print $$3}' | sed 's/;//').pcm

$(BUILD_DIR)/%.o : $(BUILD_DIR)/%.pcm | config_file
	$(Q)$(CC) $(CPPFLAGS) $(CXXFLAGS) -fimplicit-modules -fimplicit-module-maps -fprebuilt-module-path=$(MOD_PREBUILT_DIR)/ -Wno-unused-command-line-argument -c $< -o $@

# module order dependencies
# TODO: find a better way
$(BUILD_DIR)/src/board/qemu/aarch64/debug/debug.pcm : | $(BUILD_DIR)/src/board/qemu/aarch64/debug/uart.pcm
$(BUILD_DIR)/src/board/qemu/aarch64/debug/uart.pcm : | $(BUILD_DIR)/src/lib/reg.pcm
$(BUILD_DIR)/src/board/qemu/aarch64/init/init.pcm : | $(BUILD_DIR)/src/board/qemu/aarch64/debug/debug.pcm
$(BUILD_DIR)/src/lib/heap.pcm : | $(BUILD_DIR)/src/lib/allocator/simple.pcm
$(BUILD_DIR)/src/lib/exception.pcm : | $(BUILD_DIR)/src/libcxx/string.pcm
$(BUILD_DIR)/src/libcxx/concepts.pcm : | $(BUILD_DIR)/src/libcxx/type_traits.pcm
$(BUILD_DIR)/src/lib/fmt.pcm : | $(BUILD_DIR)/src/libcxx/string.pcm $(BUILD_DIR)/src/libcxx/concepts.pcm $(BUILD_DIR)/src/board/qemu/aarch64/debug/uart.pcm
$(BUILD_DIR)/src/lib/lock/lock.pcm : | $(BUILD_DIR)/src/lib/lock/lock_aarch64.pcm
$(BUILD_DIR)/src/lib/timestamp/timestamp.pcm : | $(BUILD_DIR)/src/lib/timestamp/aarch64.pcm
$(BUILD_DIR)/src/lib/time.pcm : | $(BUILD_DIR)/src/lib/timestamp/timestamp.pcm
$(BUILD_DIR)/src/device/device.pcm : | $(BUILD_DIR)/src/libcxx/string.pcm
$(BUILD_DIR)/src/libcxx/string.pcm : | $(BUILD_DIR)/src/lib/heap.pcm
$(BUILD_DIR)/src/device/uart/pl011.pcm : | $(BUILD_DIR)/src/lib/reg.pcm $(BUILD_DIR)/src/lib/fmt.pcm


config_file: src/$(CONFIG_FILE)
	$(Q)mkdir -p $(MOD_PREBUILT_DIR)
	$(Q)mkdir -p $(BUILD_DIR)/include/
	$(Q)cp $< $(BUILD_DIR)/include/config.h

clean:
	$(Q)find $(BUILD_DIR) -name *.o | xargs rm -f
	$(Q)find $(BUILD_DIR) -name *.pcm | xargs rm -f
	$(Q)rm -f $(BUILD_DIR)/sc.elf $(BUILD_DIR)/sc.bin $(BUILD_DIR)/*.map $(BUILD_DIR)/_sc.lds
