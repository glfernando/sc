
src-y += mem.cpp
src-y += sleep.cpp
src-y += echo.cpp
src-y += loop.cpp

ifeq ($(ARCH), aarch64)
src-y += sysreg_aarch64.cpp
endif
ifeq ($(ARCH), arm)
src-y += sysreg_arm.cpp
endif

src-$(CONFIG_AARCH64_MTE) += mte.cpp
