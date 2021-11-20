/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <arch/aarch64/sysreg.h>

export module lib.cpu.arch;

export namespace lib::cpu {

void disable_irq() {
    asm volatile("msr daifset, #3");
}

void enable_irq() {
    asm volatile("msr daifclr, #3");
}

long save_and_disable_irq() {
    long flags;
    // clang-format off
    asm volatile(R"(
        mrs %0, daif
        msr daifset, #3
    )" : "=r"(flags));
    // clang-format on
    return flags;
}

void restore_irq(long flags) {
    asm volatile("msr daif, %0" ::"r"(flags));
}

unsigned id() {
    unsigned mpidr = sysreg_read(mpidr_el1);
    unsigned id = mpidr & 0xffffff;

    if (id >= 0x100) {
        unsigned cluster = (id >> 8) & 0xff;
        id = (id & 0xff) + CONFIG_CPU_AFF0_CPU_MAX * cluster;
    }

    return id;
}

}  // namespace lib::cpu
