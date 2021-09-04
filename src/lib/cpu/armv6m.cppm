/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <arch/arm/sysreg.h>

export module lib.cpu.arch;

export namespace lib::cpu {

void disable_irq() {
    asm volatile("cpsid i");
}

void enable_irq() {
    asm volatile("cpsie i");
}

unsigned long save_and_disable_irq() {
    auto flags = sysreg_read(primask);
    asm volatile("cpsid i");
    return flags;
}

void restore_irq(unsigned long flags) {
    sysreg_write(primask, flags);
}

}  // namespace lib::cpu
