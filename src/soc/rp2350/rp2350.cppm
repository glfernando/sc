/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>

export module soc.rp2350;

import soc.rp2350.address_map;
import lib.reg;

using namespace soc::rp2350::address_map;
using lib::reg::reg32;

namespace {

volatile uint32_t& reg_set(uintptr_t addr) {
    return reg32(addr + 0x2000);
}

volatile uint32_t& reg_clr(uintptr_t addr) {
    return reg32(addr + 0x3000);
}

}  // namespace

export namespace soc::rp2350 {

void early_init() {}

void init() {
    constexpr uintptr_t RESETS_RESET = RESETS_BASE + 0x00;
    constexpr uintptr_t RESETS_RESET_DONE = RESETS_BASE + 0x08;

    constexpr uintptr_t CLK_REF_CTRL = CLOCKS_BASE + 0x30;
    constexpr uintptr_t CLK_REF_DIV = CLOCKS_BASE + 0x34;
    constexpr uintptr_t CLK_REF_SELECTED = CLOCKS_BASE + 0x38;
    constexpr uintptr_t CLK_SYS_CTRL = CLOCKS_BASE + 0x3c;
    constexpr uintptr_t CLK_SYS_DIV = CLOCKS_BASE + 0x40;
    constexpr uintptr_t CLK_SYS_SELECTED = CLOCKS_BASE + 0x44;
    constexpr uintptr_t CLK_PERI_CTRL = CLOCKS_BASE + 0x48;

    constexpr uintptr_t XOSC_CTRL = XOSC_BASE + 0x00;
    constexpr uintptr_t XOSC_STATUS = XOSC_BASE + 0x04;
    constexpr uintptr_t XOSC_STARTUP = XOSC_BASE + 0x0c;

    constexpr uintptr_t PLL_SYS_CS = PLL_SYS_BASE + 0x00;
    constexpr uintptr_t PLL_SYS_PWR = PLL_SYS_BASE + 0x04;
    constexpr uintptr_t PLL_SYS_FBDIV_INT = PLL_SYS_BASE + 0x08;
    constexpr uintptr_t PLL_SYS_PRIM = PLL_SYS_BASE + 0x0c;

    constexpr uintptr_t TICKS_TIMER0_CTRL = TICKS_BASE + 0x18;
    constexpr uintptr_t TICKS_TIMER0_CYCLES = TICKS_BASE + 0x1c;

    constexpr uint32_t RESET_IO_BANK0 = 1U << 6;
    constexpr uint32_t RESET_PADS_BANK0 = 1U << 9;
    constexpr uint32_t RESET_PLL_SYS = 1U << 14;
    constexpr uint32_t RESET_TIMER0 = 1U << 23;
    constexpr uint32_t RESET_UART0 = 1U << 26;

    // Switch clk_sys and clk_ref back to safe default sources before reconfiguring XOSC/PLL
    reg_clr(CLK_SYS_CTRL) = 0x1;
    while (!(reg32(CLK_SYS_SELECTED) & 0x1)) {}
    reg_clr(CLK_REF_CTRL) = 0x3;
    while (!(reg32(CLK_REF_SELECTED) & 0x1)) {}

    // Enable 12 MHz XOSC
    reg32(XOSC_STARTUP) = 0x2f;
    reg32(XOSC_CTRL) = (0xfabU << 12) | 0xaa0U;
    while (!(reg32(XOSC_STATUS) & (1U << 31))) {}

    // Switch clk_ref to XOSC (12 MHz, divider = 1.0 in 16.16 fixed point)
    reg32(CLK_REF_DIV) = 1U << 16;
    reg32(CLK_REF_CTRL) = 0x2;
    while (!(reg32(CLK_REF_SELECTED) & (1U << 2))) {}

    // Reset and configure PLL_SYS for 150 MHz: 12 MHz / 1 * 125 / (5 * 2) = 150 MHz
    reg_set(RESETS_RESET) = RESET_PLL_SYS;
    reg_clr(RESETS_RESET) = RESET_PLL_SYS;
    while (!(reg32(RESETS_RESET_DONE) & RESET_PLL_SYS)) {}

    reg32(PLL_SYS_CS) = 1U;
    reg32(PLL_SYS_FBDIV_INT) = 125U;
    reg32(PLL_SYS_PWR) = (1U << 3) | (1U << 2);  // clear PD & VCOPD; keep POSTDIVPD & DSMPD
    while (!(reg32(PLL_SYS_CS) & (1U << 31))) {}

    reg32(PLL_SYS_PRIM) = (5U << 16) | (2U << 12);
    reg_clr(PLL_SYS_PWR) = 1U << 3;  // clear POSTDIVPD

    // Switch clk_sys to clksrc_pll_sys (150 MHz, divider = 1.0)
    reg32(CLK_SYS_DIV) = 1U << 16;
    reg32(CLK_SYS_CTRL) = 0x1;
    while (!(reg32(CLK_SYS_SELECTED) & (1U << 1))) {}
    asm volatile("isb");

    // Enable clk_peri sourced from clk_sys (150 MHz)
    reg32(CLK_PERI_CTRL) = 1U << 11;

    // Configure 1 MHz tick generator for TIMER0 from 12 MHz clk_ref
    reg32(TICKS_TIMER0_CTRL) = 0;
    reg32(TICKS_TIMER0_CYCLES) = 12;
    reg32(TICKS_TIMER0_CTRL) = 1;
    while (!(reg32(TICKS_TIMER0_CTRL) & (1U << 1))) {}

    // Release IO_BANK0, PADS_BANK0, TIMER0, and UART0 from reset
    constexpr uint32_t peri_resets =
        RESET_IO_BANK0 | RESET_PADS_BANK0 | RESET_TIMER0 | RESET_UART0;
    reg_clr(RESETS_RESET) = peri_resets;
    while ((reg32(RESETS_RESET_DONE) & peri_resets) != peri_resets) {}

    // Configure GP0 (UART0 TX) and GP1 (UART0 RX) pads: clear ISO (bit 8) and set IE (bit 6)
    reg_clr(PADS_BANK0_BASE + 0x04) = (1U << 8) | (1U << 7);
    reg_set(PADS_BANK0_BASE + 0x04) = 1U << 6;
    reg_clr(PADS_BANK0_BASE + 0x08) = (1U << 8) | (1U << 2);
    reg_set(PADS_BANK0_BASE + 0x08) = (1U << 6) | (1U << 3);

    // Mux GP0/GP1 to UART0 (FUNCSEL = 2)
    reg32(IO_BANK0_BASE + 0x04) = 2;
    reg32(IO_BANK0_BASE + 0x0c) = 2;
}

}  // namespace soc::rp2350
