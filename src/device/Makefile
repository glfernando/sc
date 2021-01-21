LOCAL_DIR = $(GET_LOCAL_DIR)

mod_srcs += $(LOCAL_DIR)/device.cppm

dirs :=
dirs += $(LOCAL_DIR)/uart
dirs += $(LOCAL_DIR)/console
dirs += $(LOCAL_DIR)/intc
dirs += $(LOCAL_DIR)/timer

include $(foreach dir,$(dirs),$(dir)/Makefile)
