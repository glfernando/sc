LOCAL_DIR = $(GET_LOCAL_DIR)

mod_srcs += $(LOCAL_DIR)/reg.cppm
mod_srcs += $(LOCAL_DIR)/heap.cppm
mod_srcs += $(LOCAL_DIR)/exception.cppm
srcs += $(LOCAL_DIR)/heap-malloc.cpp

dirs :=
dirs += $(LOCAL_DIR)/allocator

include $(foreach dir,$(dirs),$(dir)/Makefile)
