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

class lock_irqsafe : public lock {
 public:
    lock_irqsafe() : lock(), flags(0) {}
    [[gnu::always_inline]] void acquire();
    [[gnu::always_inline]] void release();

 private:
    unsigned long flags;
};

}  // namespace lib

namespace lib {

void lock::acquire() {
    val = 1;
}

void lock::release() {
    val = 0;
}

void lock_irqsafe::acquire() {
    flags = sysreg_read(primask);
    asm volatile("cpsid i");
    lock::acquire();
}

void lock_irqsafe::release() {
    lock::release();
    sysreg_write(primask, flags);
}

}  // namespace lib
