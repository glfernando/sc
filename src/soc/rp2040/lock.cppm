/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <arch/arm/sysreg.h>

export module lib.lock.arch;

export namespace lib {

class lock {
 public:
    lock() : val(0) {}

    [[gnu::always_inline]] void acquire();
    [[gnu::always_inline]] void release();

 public:
    unsigned val;
};

}  // namespace lib

namespace lib {

void lock::acquire() {
    val = 1;
}

void lock::release() {
    val = 0;
}

}  // namespace lib
