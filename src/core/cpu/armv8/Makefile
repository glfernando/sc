LOCAL_DIR = $(GET_LOCAL_DIR)

mod_srcs += $(LOCAL_DIR)/cpu.cppm
mod_srcs += $(LOCAL_DIR)/exception.cppm
mod_srcs += $(LOCAL_DIR)/common.cppm

srcs += $(LOCAL_DIR)/loader.S
srcs += $(LOCAL_DIR)/start.S
srcs += $(LOCAL_DIR)/exception_vector.S

CONFIG_CPU_AFF0_CPU_MAX ?= 8

GLOBAL_CFLAGS += -DCONFIG_CPU_AFF0_CPU_MAX=$(CONFIG_CPU_AFF0_CPU_MAX)
