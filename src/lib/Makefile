LOCAL_DIR = $(GET_LOCAL_DIR)

mod_srcs += $(LOCAL_DIR)/reg.cppm
mod_srcs += $(LOCAL_DIR)/heap.cppm
mod_srcs += $(LOCAL_DIR)/exception.cppm
mod_srcs += $(LOCAL_DIR)/fmt.cppm

srcs += $(LOCAL_DIR)/heap-malloc.cpp

dirs :=
dirs += $(LOCAL_DIR)/allocator
dirs += $(LOCAL_DIR)/lock
dirs += $(LOCAL_DIR)/timestamp

include $(foreach dir,$(dirs),$(dir)/Makefile)
