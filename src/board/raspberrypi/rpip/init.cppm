/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>
#include <stdio.h>

export module board.init;

import lib.reg;
import board.peripherals;

#define RESETS_BASE 0x4000c000
#define RESET       (RESETS_BASE + 0x00)
#define RESET_DONE  (RESETS_BASE + 0x08)

using sc::lib::reg::reg32;

volatile uint32_t& reg_clr(uintptr_t addr) {
    return reg32(addr + 0x3000);
}

volatile uint32_t& reg_set(uintptr_t addr) {
    return reg32(addr + 0x2000);
}

// TODO: move it to a SoC module
static void soc_init() {
    reg_set(RESET) = ~0x3240;
    reg_clr(RESET) = 0x01ffffff & ~(0x01c38001);

    while (~reg32(RESET_DONE) & (0x01ffffff & ~(0x01c38001))) {}

    // WDT
    reg32(0x4005802c) = (1 << 9) | 12;

    reg32(0x40008078) = 0;

    // XOSC
    reg32(0x40024000) = 0xd1eaa0;
    reg32(0x4002400c) = 0x2f;
    reg32(0x40024000) = 0xfab << 12;

    while (!(reg32(0x40024004) & (1 << 31))) {}

    reg32(0x40008030) = 0x2;

    for (int i = 0; i < 10000; ++i) {
        asm volatile("nop");
        asm volatile("yield");
    }

    reg32(0x4000803c) = 0;

    reg32(0x40008080) = 12000;
    reg32(0x40008090) = 10;
    reg32(0x40008094) = 9;

    while (!(reg32(0x40008098) & (1 << 4))) {}

    // SYS PLL
    // 0x40028000
    reg_set(0x4000c000) = 1 << 12;
    reg_clr(0x4000c000) = 1 << 12;
    while (!(reg32(0x4000c008) & (1 << 12))) {}

    reg32(0x40028004) = 0xffffffff;
    reg32(0x40028008) = 125;
    reg_clr(0x40028004) = (1 << 0) | (1 << 5);
    while (!(reg32(0x40028000) & (1 << 31))) {}
    reg32(0x4002800c) = (6 << 16) | (2 << 12);
    reg_clr(0x40028004) = (1 << 3);

    // sys clk to pll
    reg32(0x4000803c) = 1;

    asm volatile("isb");
    for (int i = 0; i < 100; ++i) {
        asm volatile("nop");
        asm volatile("yield");
    }
    // peri clk
    reg32(0x40008048) = 1 << 11;

    reg_clr(0x4000c000) = (1 << 22) | (1 << 8) | (1 << 5);
    while (!(reg32(0x4000c008) & ((1 << 22) | (1 << 8) | (1 << 5)))) {}
    // gpio pads

    // gpio tx/rx func
    reg32(0x40014004) = 2;
    reg32(0x4001400c) = 2;
}

export namespace sc::board {

void early_init() {}

void init() {
    soc_init();
    peripherals::init();
}

}  // namespace sc::board
