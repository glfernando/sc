/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <arch/aarch64/sysreg.h>
#include <errcodes.h>
#include <stdint.h>

#include "exception_vector.h"

export module core.cpu.arch;

export import core.cpu.armv8.common;

import core.cpu.armv8.exception;
import lib.fmt;

using lib::fmt::println;

export using cpu_irq_handler = void(*)(int vec, void* data);

namespace core::cpu {

// 1:1 mmu mapping of the first 4GB
static uint64_t xlate_table[4096 / 8] __attribute__((aligned(4096))) = {
    // clang-format off
    [0] = 0x00000421, // 1GB device memory
    [1] = 0x4000070d, // 1GB writeback memory
    [2] = 0x8000070d, // 1GB writeback memory
    [3] = 0xc000070d, // 1GB writeback memory
    // clang-format off
};

static cpu_irq_handler cpu_handler;
static void* cpu_handler_data;

static int cpu_exception_handler(armv8::exception::regs*) {
    if (cpu_handler)
        cpu_handler(0, cpu_handler_data);
    else
        println("no cpu irq handler");
    return 0;
}

static void mmu_init() {
    sysreg_write(tcr_el1, 0x0000'3520UL);
    sysreg_write(mair_el1, 0xff44'0400UL);

    sysreg_write(ttbr0_el1, xlate_table);

    unsigned long val = sysreg_read(sctlr_el1);
    // enable mmu and D/I caches
    val |= (1 << 12) | (1 << 2) | 1;
    sysreg_write(sctlr_el1, val);

    asm volatile("dsb sy");
    asm volatile("isb");
}

}  // namespace core::cpu

export namespace core::cpu {

void early_init() {
    mmu_init();
}

void init() {
    armv8::exception::init();
    register_exception_handler(EXCEPTION_TYPE_SPX_IRQ, cpu_exception_handler);
}

void register_irq_handler(cpu_irq_handler handler, void* data) {
    cpu_handler = handler;
    cpu_handler_data = data;
}

}  // namespace core::cpu
