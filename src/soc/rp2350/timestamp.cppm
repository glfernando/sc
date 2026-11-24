/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>

export module lib.timestamp.arch;

import lib.reg;
import soc.rp2350.address_map;

using lib::reg::reg32;
using soc::rp2350::address_map::TIMER0_BASE;

constexpr uintptr_t TIMERHR = TIMER0_BASE + 0x08;
constexpr uintptr_t TIMERLR = TIMER0_BASE + 0x0c;

export namespace lib::timestamp {

uint64_t ticks() {
    uint32_t lo = reg32(TIMERLR);
    uint32_t hi = reg32(TIMERHR);
    return (static_cast<uint64_t>(hi) << 32) | lo;
}

uint64_t freq() {
    return 1'000'000;
}

}  // namespace lib::timestamp
