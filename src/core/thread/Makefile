LOCAL_DIR = $(GET_LOCAL_DIR)

mod_srcs += $(LOCAL_DIR)/thread.cppm

ifeq ($(CPU), armv8)
mod_srcs += $(LOCAL_DIR)/armv8.cppm
else ifeq ($(CPU), armv6m)
mod_srcs += $(LOCAL_DIR)/armv6m.cppm
endif
