/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdio.h>

export module board.init;

import board.debug;

export namespace sc::board {

void init() {
    sc::board::debug::uart::init();
    printf_set_putchar_func(sc::board::debug::uart::putchar);
}

}
