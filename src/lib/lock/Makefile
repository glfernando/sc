
src-y += lock.cppm

ifeq ($(ARCH), aarch64)
src-y += lock_aarch64.cppm
endif

ifeq ($(CPU), armv8m)
src-y += lock_armv8m.cppm
endif

src-$(CONFIG_LOCK_DUMMY) += lock_dummy.cppm
