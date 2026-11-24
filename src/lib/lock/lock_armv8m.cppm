/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module lib.lock.arch;

export namespace lib {

class lock {
 public:
    constexpr lock() : val(0) {}

    [[gnu::always_inline]] void acquire();
    [[gnu::always_inline]] void release();

 public:
    unsigned val __attribute__((aligned(4)));
};

}  // namespace lib

namespace lib {

void lock::acquire() {
    unsigned x;
    unsigned fail;
    unsigned one = 1;

    while (true) {
        asm volatile("ldaex %0, [%1]" : "=r"(x) : "r"(&val) : "memory");
        if (x) {
            asm volatile("wfe");
            continue;
        }
        asm volatile("strex %0, %1, [%2]" : "=&r"(fail) : "r"(one), "r"(&val) : "memory");
        if (!fail)
            break;
    }
}

void lock::release() {
    unsigned zero = 0;
    asm volatile("stl %0, [%1]\ndsb\nsev\n" :: "r"(zero), "r"(&val) : "memory");
}

}  // namespace lib
