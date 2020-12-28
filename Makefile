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

CPPFLAGS = -std=c++2a -fno-rtti -nodefaultlibs -nostdinc++ -fno-builtin -flto -fpie -Oz -Wall -g
CPPFLAGS += -ffunction-sections -fdata-sections -Wno-deprecated-volatile
CPPFLAGS += -Iinclude
CPPFLAGS += -include config.h
OBJCPYFLAGS = -O binary --strip-all
LDFLAGS = --gc-sections --pie

TARGET ?= qemu-aarch64
MOD_PREBUILT_DIR ?= prebuilts

srcs :=
mod_srcs :=
dirs :=
GLOBAL_CPPFLAGS :=

include make/utils.mk
include target/$(TARGET).mk

# include top directories
include src/Makefile
include src/$(BOARD_PATH)/Makefile

# use default linker script if none was set by target makefile
LINKER_SCRIPT ?= sc_linker.lds

CPPFLAGS += $(GLOBAL_CPPFLAGS)

cpp_srcs = $(filter %.cpp, $(srcs))
asm_srcs = $(filter %.S, $(srcs))
cpp_objs = $(patsubst %.cpp, %.o, $(cpp_srcs))
asm_objs = $(patsubst %.S, %.o, $(asm_srcs))
mod_pcms = $(patsubst %.cppm, %.pcm, $(mod_srcs))
mod_objs = $(patsubst %.cppm, %.o, $(mod_srcs))

objs = $(cpp_objs) $(asm_objs) $(mod_objs)

.PHONY: clean modules config_file

all: sc.bin

sc.bin: sc.elf | modules
	$(Q)$(OBJCOPY) $(OBJCPYFLAGS) $< $@

sc.elf : $(objs) | _sc.lds
	$(Q)$(LD) $(LDFLAGS) -T _sc.lds -o $@ $^

_sc.lds : $(LINKER_SCRIPT)
	$(Q)$(CC) $(CPPFLAGS) -E -x assembler-with-cpp -P -o $@ $<

%.o : %.S | config_file
	$(Q)$(CC) $(CPPFLAGS) -c $< -o $@

%.o : %.cpp | modules config_file
	$(Q)$(CC) $(CPPFLAGS) -fimplicit-modules -fimplicit-module-maps -fprebuilt-module-path=$(MOD_PREBUILT_DIR)/ -c $< -o $@

modules: $(mod_pcms)

%.pcm : %.cppm | config_file
	$(Q)$(CC) $(CPPFLAGS) -fimplicit-modules -fimplicit-module-maps -fmodules -fprebuilt-module-path=$(MOD_PREBUILT_DIR)/ --precompile $< -o $@
	$(Q)cp $@ $(MOD_PREBUILT_DIR)/$(shell grep "^export module" $< | awk '{print $$3}' | sed 's/;//').pcm

%.o : %.pcm | config_file
	$(Q)$(CC) $(CPPFLAGS) -fimplicit-modules -fimplicit-module-maps -fprebuilt-module-path=$(MOD_PREBUILT_DIR)/ -Wno-unused-command-line-argument -c $< -o $@

# module order dependencies
# TODO: find a better way
src/board/qemu/aarch64/debug/debug.pcm : | src/board/qemu/aarch64/debug/uart.pcm
src/board/qemu/aarch64/debug/uart.pcm : | src/lib/reg.pcm
src/board/qemu/aarch64/init/init.pcm : | src/board/qemu/aarch64/debug/debug.pcm


config_file: src/$(CONFIG_FILE)
	$(Q)mkdir -p $(MOD_PREBUILT_DIR)
	$(Q)mkdir -p include/
	$(Q)cp $< include/config.h

clean:
	$(Q)find . -name *.o | xargs rm -f
	$(Q)find . -name *.pcm | xargs rm -f
	$(Q)rm -f sc.elf sc.bin *.map _sc.lds
