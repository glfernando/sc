LOCAL_DIR = $(GET_LOCAL_DIR)

dirs :=
dirs += $(LOCAL_DIR)/core
dirs += $(LOCAL_DIR)/device
dirs += $(LOCAL_DIR)/lib
dirs += $(LOCAL_DIR)/libc
dirs += $(LOCAL_DIR)/libcxx
dirs += $(LOCAL_DIR)/libcxxabi

include $(foreach dir,$(dirs),$(dir)/Makefile)
