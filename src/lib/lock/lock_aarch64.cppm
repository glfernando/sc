/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module lib.lock.aarch64;

export namespace sc::lib {

class lock {
 public:
    lock() : val(0) {}

    [[gnu::always_inline]] void acquire();
    [[gnu::always_inline]] void release();

 public:
    volatile unsigned val __attribute__((aligned(8)));
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

void lock::acquire()
{
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

void lock::release() { asm volatile("stlr wzr, [%0]" ::"r"(&val)); }

void lock_irqsafe::acquire()
{
    asm volatile(
        "mrs %0, daif\n"
        "msr daifset, #3\n"
        : "=r"(flags));
    lock::acquire();
}

void lock_irqsafe::release()
{
    lock::release();
    asm volatile("msr daif, %0" ::"r"(flags));
}

}  // namespace sc::lib
