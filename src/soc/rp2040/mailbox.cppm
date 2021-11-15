/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module soc.rp2040.mailbox;

#include <stdint.h>

import lib.reg;
import soc.rp2040.address_map;

using lib::reg::reg32;
using namespace soc::rp2040::address_map;

namespace {

// clang-format off
enum regs : uint32_t {
    FIFO_ST = 0x50,
    FIFO_WR = 0x54,
    FIFO_RD = 0x58,
};
// clang-format on

enum FIFO_ST_bits : uint32_t {
    VLD = 1 << 0,
    RDY = 1 << 1,
    WOF = 1 << 2,
    ROE = 1 << 3,
};

volatile uint32_t& reg(uint32_t offset) {
    return reg32(SIO_BASE + offset);
}

};  // namespace

export namespace soc::rp2040::mailbox {

uint32_t read() noexcept {
    // TODO: support for interrupt
    while (!(reg(FIFO_ST) & VLD)) {
        asm volatile("sev");
    }
    return reg(FIFO_RD);
}

void write(uint32_t val) noexcept {
    while (!(reg(FIFO_ST) & RDY)) {}
    reg(FIFO_WR) = val;
    asm volatile("sev");
}

void flush() noexcept {
    while (reg(FIFO_ST) & VLD) {
        [[maybe_unused]] auto _ = reg(FIFO_RD);
    }
}

}  // namespace soc::rp2040::mailbox
