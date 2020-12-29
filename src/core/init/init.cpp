/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

import board.init;
import board.debug;
import lib.heap;

using sc::board::debug::uart::puts;

extern void (*__init_array_start[])();
extern void (*__init_array_end[])();

void init_array()
{
    for (void (**f)() = __init_array_start; f < __init_array_end; ++f)
        (*f)();
}

extern "C" [[noreturn]] void init()
{
    sc::board::init();

    sc::lib::heap::init();

    init_array();

    puts("Welcome to SC\n");
    for (;;) {}
}
