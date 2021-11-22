/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <arch/aarch64/sysreg.h>
#include <stdint.h>
#include <stdio.h>

export module board.init;

import board.debug;
import board.peripherals;
import arch.aarch64.smc;
import lib.fmt;
import core.thread;

using lib::fmt::println;

// TODO: make this configurable
constexpr size_t SMP_CPUS = 2;

namespace {

static uint64_t vbar;
static uint64_t mair;
static uint64_t tcr;
static uint64_t ttbr0;
static uint64_t sctlr;

[[noreturn]] void sec_init() {
    sysreg_write(vbar_el1, vbar);
    sysreg_write(mair_el1, mair);
    sysreg_write(tcr_el1, tcr);
    sysreg_write(ttbr0_el1, ttbr0);
    asm volatile("dsb sy");
    asm volatile("isb");

    sysreg_write(sctlr_el1, sctlr);

    asm volatile("dsb sy");
    asm volatile("isb");

    board::peripherals::init_sec();
    core::thread::init();

    // should not return here
    for (;;) {
        asm volatile("wfi");
    }
}

__attribute__((naked)) void sec_entry() {
    asm volatile(R"(
.equ SCR_EL3_NS, (1 << 0)
.equ SCR_EL3_RW, (1 << 10)
.equ HCR_EL2_RW, (1 << 31)

        mov     x4, x0

        // switch from ELX to EL1
        mrs     x0, CurrentEL
        tbz     x0, 2, 2f       // test for EL2
        tbz     x0, 3, 1f       // test for EL1

        // configure EL3 registers
        msr     cptr_el3, xzr
        mrs     x0, scr_el3
        orr     x0, x0, SCR_EL3_RW
        orr     x0, x0, SCR_EL3_NS
        msr     scr_el3, x0

        // configure EL3 -> EL1 switch
        mov     x0, #((0b1111 << 6) | 0b0101)   // EL1h + DAIF  masked
        msr     spsr_el3, x0
        adr     x0, 1f
        msr     elr_el3, x0

2:
        // initialize sctlr_el1 reg before entering EL1
        msr     sctlr_el1, xzr

        // configure EL2 registers
        mrs     x0, hcr_el2
        orr     x0, x0, HCR_EL2_RW
        msr     hcr_el2, x0
        msr     cptr_el2, xzr

        // configure EL2 -> EL1 switch
        mov     x0, #((0b1111 << 6) | 0b0101)   // EL1h + DAIF masked
        msr     spsr_el2, x0
        adr     x0, 1f
        msr     elr_el2, x0

        // switch to EL1
        isb
        eret
1:
        // we are running at EL1
        mov     x0, (3 << 20)
        msr     cpacr_el1, x0
        isb

        // setup stack
        msr     spsel, #1
        mov     sp, x4

        b %0
    )" ::"S"(sec_init));
}

}  // namespace

export namespace board {

void early_init() {
    // initialize debug console
    board::debug::uart::init();
    printf_set_putchar_func(board::debug::uart::putchar);
}

void init() {
    // initialize peripherals
    peripherals::init();
}

void late_init() {
    vbar = sysreg_read(vbar_el1);
    mair = sysreg_read(mair_el1);
    tcr = sysreg_read(tcr_el1);
    ttbr0 = sysreg_read(ttbr0_el1);
    sctlr = sysreg_read(sctlr_el1);

    constexpr int STACK_SIZE = 4096;

    for (size_t i = 1; i != SMP_CPUS; ++i) {
        uintptr_t stack = reinterpret_cast<uintptr_t>(new uint8_t[STACK_SIZE] + STACK_SIZE);
        auto r = aarch64::smc(0xc400'0003, i, reinterpret_cast<unsigned long>(sec_entry), stack);
        if (r) {
            println("failed to initialize CPU{} {:#x}", i, r);
        }
    }
}

}  // namespace board
