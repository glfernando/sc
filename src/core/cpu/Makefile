LOCAL_DIR = $(GET_LOCAL_DIR)

mod_srcs += $(LOCAL_DIR)/cpu.cppm

dirs :=

ifeq ($(CPU), armv8)
dirs += $(LOCAL_DIR)/armv8
endif

include $(foreach dir,$(dirs),$(dir)/Makefile)
