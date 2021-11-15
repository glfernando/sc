/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module soc.rp2040.hwspinlock;

#include <stdint.h>

import soc.rp2040.address_map;
import lib.reg;

using namespace soc::rp2040::address_map;
using lib::reg::reg32;

constexpr uint32_t SPINLOCK0 = 0x100;

export namespace soc::rp2040 {

class hwspinlock {
 public:
    constexpr hwspinlock(unsigned index) : addr(SIO_BASE + SPINLOCK0 + ((index * 4) % 32)) {}

    bool try_acquire() { return reg32(addr); }

    void acquire() {
        while (!try_acquire()) {}
    }

    void release() { reg32(addr) = 1; }

 private:
    const uintptr_t addr;
};

};  // namespace soc::rp2040
