/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdio.h>

export module board.init;

import board.debug;
import board.peripherals;

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

void late_init() {}

}  // namespace board
