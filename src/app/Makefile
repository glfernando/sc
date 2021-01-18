LOCAL_DIR = $(GET_LOCAL_DIR)

dirs :=
dirs += $(LOCAL_DIR)/shell
dirs += $(LOCAL_DIR)/cmds

ifeq ($(CONFIG_INCLUDE_TESTS), y)
dirs += $(LOCAL_DIR)/tests
endif

include $(foreach dir,$(dirs),$(dir)/Makefile)
