/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>
#include <stdio.h>

export module soc.rp2040;

import soc.rp2040.address_map;
import soc.rp2040.hwspinlock;
import soc.rp2040.mailbox;
import soc.rp2040.bootrom;
import soc.rp2040.rtc;
import lib.reg;

#define RESET      (soc::rp2040::address_map::RESETS_BASE + 0x00)
#define RESET_DONE (soc::rp2040::address_map::RESETS_BASE + 0x08)

using lib::reg::reg32;

volatile uint32_t& reg_clr(uintptr_t addr) {
    return reg32(addr + 0x3000);
}

volatile uint32_t& reg_set(uintptr_t addr) {
    return reg32(addr + 0x2000);
}

export namespace soc::rp2040 {

void early_init() {
    bootrom::init();
}

// TODO: create proper clk, gpio, reset, etc drivers

void init() {
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

    // USB PLL
    // 0x4002c000
    reg_set(0x4000c000) = 1 << 13;
    reg_clr(0x4000c000) = 1 << 13;
    while (!(reg32(0x4000c008) & (1 << 13))) {}

    reg32(0x4002c000) = 0x1;
    reg32(0x4002c008) = 0x28;
    reg32(0x4002c004) = 0xffffffff;
    reg_clr(0x4002c004) = (1 << 0) | (1 << 5);
    while (!(reg32(0x4002c000) & (1 << 31))) {}
    reg32(0x4002c00c) = (5 << 16) | (2 << 12);
    reg_clr(0x4002c004) = (1 << 3);

    // sys clk to pll
    reg32(0x4000803c) = 1;

    asm volatile("isb");
    for (int i = 0; i < 100; ++i) {
        asm volatile("nop");
        asm volatile("yield");
    }

    // enable usb clk
    reg32(0x40008054) = 1 << 11;

    // adc clk
    reg32(0x40008060) = 1 << 11;

    // rtc clk
    reg32(0x4000806c) = 1 << 11;
    reg32(0x40008070) = 0x00040000;

    // peri clk
    reg32(0x40008048) = 1 << 11;

    reg_clr(0x4000c000) = (1 << 22) | (1 << 8) | (1 << 5);
    while (!(reg32(0x4000c008) & ((1 << 22) | (1 << 8) | (1 << 5)))) {}
    // gpio pads

    // gpio tx/rx func
    reg32(0x40014004) = 2;
    reg32(0x4001400c) = 2;

    hwspinlock::init();
    mailbox::init();
    rtc::init();
}

}  // namespace soc::rp2040
