/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>

export module soc.rp2040.reset;

import lib.reg;
import soc.rp2040.address_map;

using lib::reg::reg32;
using namespace soc::rp2040::address_map;

namespace {

constexpr unsigned RTC_CLK_FREQ = 46875;

// clang-format off
enum reg_offset : uint32_t {
    RESET       = 0x00,
    RESET_DONE  = 0x08,
};
// clang-format on

static volatile uint32_t& reg(uint32_t offset) {
    return reg32(RESETS_BASE + offset);
}

}  // namespace

export namespace soc::rp2040::reset {

// clang-format off
enum {
    ADC         = 1 << 0,
    BUSCTRL     = 1 << 1,
    DMA         = 1 << 2,
    I2C0        = 1 << 3,
    I2C1        = 1 << 4,
    IO_BANK0    = 1 << 5,
    IO_QSPI     = 1 << 6,
    JTAG        = 1 << 7,
    PADS_BANK0  = 1 << 8,
    PADS_QSPI   = 1 << 9,
    PIO0        = 1 << 10,
    PIO1        = 1 << 11,
    PLL_SYS     = 1 << 12,
    PLL_USB     = 1 << 13,
    PWM         = 1 << 14,
    RTC         = 1 << 15,
};
// clang-format on

void set(uint32_t mask) {
    reg(RESET) |= mask;
}

void clear(uint32_t mask, bool wait = false) {
    reg(RESET) &= ~mask;
    if (wait) {
        while (~reg(RESET_DONE) & mask) {}
    }
}

}  // namespace soc::rp2040::reset
