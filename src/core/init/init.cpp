/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

import board.init;
import board.debug;
import lib.heap;

using sc::board::debug::uart::puts;

extern "C" [[noreturn]] void init()
{
    sc::board::init();

    sc::lib::heap::init();

    puts("Welcome to SC\n");
    for (;;) {}
}
