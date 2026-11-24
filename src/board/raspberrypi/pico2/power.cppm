/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>

export module board.power;

import lib.reg;

using lib::reg::reg32;

export namespace board {

[[noreturn]] void reboot(int) {
    // SCB->AIRCR: VECTKEY (0x5fa) | SYSRESETREQ (bit 2)
    reg32(0xe000ed0c) = (0x5faU << 16) | (1U << 2);
    asm volatile("dsb");
    for (;;)
        asm volatile("wfi");
}

[[noreturn]] void poweroff(int = 0) {
    for (;;)
        asm volatile("wfi");
}

}  // namespace board
