/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <arch/aarch64/sysreg.h>
#include <errcodes.h>
#include <libunwind.h>
#include <stdint.h>

#include "exception_vector.h"

export module core.cpu.armv8.exception;

import lib.fmt;
import core.cpu.armv8.common;
import lib.backtrace;

using lib::backtrace;
using lib::fmt::print;
using lib::fmt::println;

export namespace core::cpu::armv8::exception {

struct regs {
    unsigned long r[31];
    unsigned long sp;
    // FP/vector registers
    uint64_t v[32][2];
    uint64_t fpsr;
    uint64_t fpcr;
};

using exception_handler_t = int (*)(regs* regs);

}  // namespace core::cpu::armv8::exception

namespace core::cpu::armv8::exception {

static exception_handler_t handlers[EXCEPTION_TYPE_MAX];

static void fill_unw_context(unw_context_t& uc, regs* regs) {
    // copy x0 - x30
    for (auto i = 0; i < 31; ++i)
        uc.data[i] = regs->r[i];

    uc.data[31] = regs->sp;
    uc.data[32] = sysreg_read(elr_el1);
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
    unw_context_t uc;
    fill_unw_context(uc, regs);
    backtrace(&uc);

    asm volatile("b .");
    return 0;
}

}  // namespace core::cpu::armv8::exception

export namespace core::cpu::armv8::exception {

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

int register_exception_handler(int type, exception_handler_t handler) {
    if (type >= EXCEPTION_TYPE_MAX)
        return ERR_INVALID_ARGS;
    handlers[type] = handler;
    return 0;
}

}  // namespace core::cpu::armv8::exception
