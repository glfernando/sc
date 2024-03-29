/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module lib.lock.arch;

export namespace lib {

class lock {
 public:
    constexpr lock() : val(0) {}

    [[gnu::always_inline]] void acquire();
    [[gnu::always_inline]] void release();

 public:
    unsigned val __attribute__((aligned(8)));
};

}  // namespace lib

namespace lib {

void lock::acquire() {
    unsigned fail;

    asm volatile("sevl");
    while (1) {
        unsigned x;
        unsigned one = 1;
        asm volatile(
            "wfe\n"
            "ldaxr    %w0, [%1]"
            : "=r"(x)
            : "r"(&val));
        if (x)
            continue;
        asm volatile("stxr     %w0, %w1, [%2]" : "=r"(fail) : "r"(one), "r"(&val));
        if (!fail)
            break;
    }
}

void lock::release() {
    asm volatile("stlr wzr, [%0]" ::"r"(&val));
}

}  // namespace lib
