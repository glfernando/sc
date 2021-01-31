/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module lib.lock.arch;

export namespace sc::lib {

class lock {
 public:
    lock() : val(0) {}

    [[gnu::always_inline]] void acquire();
    [[gnu::always_inline]] void release();

 public:
    unsigned val __attribute__((aligned(8)));
};

class lock_irqsafe : public lock {
 public:
    lock_irqsafe() : lock(), flags(0) {}
    [[gnu::always_inline]] void acquire();
    [[gnu::always_inline]] void release();

 private:
    unsigned long flags;
};

}  // namespace sc::lib

namespace sc::lib {

void lock::acquire() {
    val = 1;
}

void lock::release() {
    val = 0;
}

void lock_irqsafe::acquire() {
    flags = 1;
    val = 1;
}

void lock_irqsafe::release() {
    lock::release();
    flags = 0;
}

}  // namespace sc::lib
