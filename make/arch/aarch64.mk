
ARCH := aarch64
AARCH64_MARCH ?= armv8.5a

GLOBAL_CPPFLAGS += -DAARCH64 -target aarch64-unknown-none

ifeq ($(CONFIG_AARCH64_MTE), y)
GLOBAL_CPPFLAGS += -march=$(AARCH64_MARCH)+memtag
GLOBAL_CPPFLAGS += -fsanitize=memtag
GLOBAL_CPPFLAGS += -DCONFIG_AARCH64_MTE
else
GLOBAL_CPPFLAGS += -march=$(AARCH64_MARCH)
endif
