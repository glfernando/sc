LOCAL_DIR = $(GET_LOCAL_DIR)

mod_srcs += $(LOCAL_DIR)/device.cppm

dirs :=
dirs += $(LOCAL_DIR)/uart

include $(foreach dir,$(dirs),$(dir)/Makefile)
