LOCAL_DIR = $(GET_LOCAL_DIR)

dirs :=
dirs += $(LOCAL_DIR)/src

GLOBAL_CFLAGS += -I$(LOCAL_DIR)/include

include $(foreach dir,$(dirs),$(dir)/Makefile)
