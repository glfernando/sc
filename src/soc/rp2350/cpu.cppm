/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module lib.cpu.soc;

import soc.rp2350.address_map;
import lib.reg;

using namespace soc::rp2350::address_map;
using lib::reg::reg32;

constexpr unsigned CPUID = 0x00;

export namespace lib::cpu {

unsigned id() {
    return reg32(SIO_BASE + CPUID);
}

}  // namespace lib::cpu
