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
import lib.fmt;

export namespace sc::board {

void early_init() {
    // initialize debug console
    sc::board::debug::uart::init();
    printf_set_putchar_func(sc::board::debug::uart::putchar);
}

void init() {
    // initialize peripherals
    peripherals::init();
    sc::lib::fmt::register_console(&peripherals::default_console());
}

}  // namespace sc::board
