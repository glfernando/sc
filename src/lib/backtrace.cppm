/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <libunwind.h>

export module lib.backtrace;

import lib.fmt;

using lib::fmt::println;

export namespace lib {

void backtrace(unw_context_t* uc = nullptr) {
    unw_cursor_t cursor;
    unw_context_t luc;

    if (!uc) {
        unw_getcontext(&luc);
        uc = &luc;
    }

    unw_init_local(&cursor, uc);

    do {
        unw_word_t ip;
        unw_get_reg(&cursor, UNW_REG_IP, &ip);
        // TODO: translate address to symbol name
#if __UINTPTR_WIDTH__ == 64
        println("[<{#016x}>]", ip);
#else
        println("[<{#08x}>]", ip);
#endif
    } while (unw_step(&cursor) > 0);
}

}  // namespace lib
