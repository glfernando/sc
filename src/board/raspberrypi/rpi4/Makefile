
LOCAL_DIR = $(GET_LOCAL_DIR)

mod_srcs += $(LOCAL_DIR)/init.cppm
mod_srcs += $(LOCAL_DIR)/peripherals.cppm
mod_srcs += $(LOCAL_DIR)/power.cppm

dirs := $(LOCAL_DIR)/debug

include $(foreach dir,$(dirs),$(dir)/Makefile)
