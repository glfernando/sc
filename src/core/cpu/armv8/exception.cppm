/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <arch/aarch64/sysreg.h>
#include <stdint.h>

#include "exception_vector.h"

export module core.cpu.armv8.exception;

import lib.fmt;

using sc::lib::fmt::print;
using sc::lib::fmt::println;

export namespace core::cpu::arvm8::exception {

struct regs {
    unsigned long r[31];
    unsigned long sp;
    // FP/vector registers
    uint64_t v[32][2];
    uint64_t fpsr;
    uint64_t fpcr;
};

using exception_handler_t = int (*)(regs* regs);

}  // namespace core::cpu::arvm8::exception

namespace core::cpu::arvm8::exception {

static exception_handler_t handlers[EXCEPTION_TYPE_MAX];

struct stack_frame {
    stack_frame* next;
    unsigned long lr;
};

void print_backtrace(unsigned long fp) {
    for (auto f = reinterpret_cast<stack_frame*>(fp); f; f = f->next)
        println("[<{:016x}>]", f->lr);
}

int default_exception_handler(regs* regs) {
    println("ESR         {:#08x}    FAR {:#016x}    ELR {:#016x}\n", sysreg_read(esr_el1),
            sysreg_read(far_el1), sysreg_read(elr_el1));

    // print general purpose registers
    for (int i = 0; i < 10; ++i) {
        if (i % 4)
            print("    ");
        print(" X{} {:#016x}", i, regs->r[i]);
        if (i % 4 == 3)
            print("\n");
    }
    for (int i = 10; i < 30; ++i) {
        if (i % 4)
            print("    ");
        print("X{} {:#016x}", i, regs->r[i]);
        if (i % 4 == 3)
            print("\n");
    }

    println("     LR {:#016x}     SP {:#016x}", regs->r[30], regs->sp);

    println("\nbacktrace:");
    print_backtrace(regs->r[29]);

    asm volatile("b .");
}

}  // namespace core::cpu::arvm8::exception

export namespace core::cpu::arvm8::exception {

unsigned cpu_id() {
    unsigned mpidr = sysreg_read(mpidr_el1);
    unsigned id = mpidr & 0xffffff;

    if (id >= 0x100) {
        unsigned cluster = (id >> 8) & 0xff;
        id = (id & 0xff) + CONFIG_CPU_AFF0_CPU_MAX * cluster;
    }

    return id;
}

extern "C" uint8_t exception_base[];
extern "C" int exception_handler(int type, regs* regs) {
    if (handlers[type])
        return handlers[type](regs);

    println("unhandled exception ({}) on cpu {}", type, cpu_id());
    return default_exception_handler(regs);
}

void init() {
    // init exception vector
    sysreg_write(vbar_el1, exception_base);
}

}  // namespace core::cpu::arvm8::exception
