/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <arch/aarch64/sysreg.h>
#include <stdint.h>
#include <stdio.h>

export module board.init;

import board.debug;
import board.peripherals;
import lib.reg;
import lib.fmt;

using sc::lib::reg::reg32;

static uint64_t xlate_table[4096 / 8] __attribute__((aligned(4096))) = {
    [0] = 0x0000'070D,
    [1] = 0x4000'070D,
    [2] = 0x8000'0421,
    [3] = 0xC000'0421,
};

export namespace sc::board {

void early_init() {
    // init MMU, do that spin locks work
    sysreg_write(tcr_el1, 0x1'0000'351c);
    sysreg_write(mair_el1, 0xff44'0400UL);

    sysreg_write(ttbr0_el1, reinterpret_cast<unsigned long>(xlate_table));

    // enable cache
    unsigned long r = sysreg_read(sctlr_el1);
    r |= (1 << 12) | 0x5;
    r &= ~(1 << 1);

    sysreg_write(sctlr_el1, r);

    asm volatile("dsb sy");
    asm volatile("isb");

    // configure uart gpios
    uintptr_t GPIO_START = 0xfe20'0000;
    uint32_t val = reg32(GPIO_START + 0x4);
    val &= ~(0x7 << 15);
    val |= 0x4 << 15;
    val &= ~(0x7 << 12);
    val |= 0x4 << 12;
    reg32(GPIO_START + 0x4) = val;

    // disable uart pin PUD
    reg32(GPIO_START + 0xe4) = 0;

    // initialize debug console
    sc::board::debug::uart::init();
    printf_set_putchar_func(sc::board::debug::uart::putchar);
}

void init() {
    // initialize peripherals
    sc::board::peripherals::init();
    sc::lib::fmt::register_console(&peripherals::default_console());
}

}  // namespace sc::board
