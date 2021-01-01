# SPDX-License-Identifier: BSD-3-Clause

LOCAL_DIR = $(GET_LOCAL_DIR)

srcs += $(LOCAL_DIR)/UnwindLevel1.c
srcs += $(LOCAL_DIR)/UnwindRegistersSave.S
srcs += $(LOCAL_DIR)/UnwindRegistersRestore.S
srcs += $(LOCAL_DIR)/libunwind.cpp

GLOBAL_CFLAGS += -D_LIBUNWIND_IS_BAREMETAL -D_LIBUNWIND_HAS_NO_THREADS -Wno-return-type -Wno-undef -DNDEBUG -Wno-unused-parameter

