
LOCAL_DIR = $(GET_LOCAL_DIR)

mod_srcs += $(LOCAL_DIR)/peripherals.cppm

dirs := $(LOCAL_DIR)/init
dirs += $(LOCAL_DIR)/debug

include $(foreach dir,$(dirs),$(dir)/Makefile)
