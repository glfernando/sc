/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module board.power;

export namespace board {

[[noreturn]] void reboot(int) {
    // TODO: implement
    for (;;)
        asm volatile("wfi");
}

[[noreturn]] void poweroff(int exit_code = 0) {
    // TODO: implement
    for (;;)
        asm volatile("wfi");
}

}  // namespace board
