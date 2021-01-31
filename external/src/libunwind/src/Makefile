# SPDX-License-Identifier: BSD-3-Clause

src-y += UnwindLevel1.c UnwindRegistersSave.S UnwindRegistersRestore.S libunwind.cpp
src-y += Unwind-EHABI.cpp

GLOBAL_CFLAGS += -D_LIBUNWIND_IS_BAREMETAL -D_LIBUNWIND_HAS_NO_THREADS -Wno-return-type -Wno-undef -DNDEBUG -Wno-unused-parameter

