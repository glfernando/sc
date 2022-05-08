# SPDX-License-Identifier: BSD-3-Clause

src-y += UnwindLevel1.cpp UnwindRegistersSave.S UnwindRegistersRestore.S libunwind.cpp
src-y += Unwind-EHABI.cpp

MODULE_CPPFLAGS += -D_LIBUNWIND_IS_BAREMETAL -D_LIBUNWIND_HAS_NO_THREADS -Wno-return-type -Wno-undef -DNDEBUG -Wno-unused-parameter -Wno-unused-function

CLANG_VERSION := $(shell $(CC) -dumpversion)
ifeq (13.0.0,$(firstword $(sort $(CLANG_VERSION) 13.0.0)))
MODULE_CPPFLAGS += -Wno-unused-but-set-variable
endif
