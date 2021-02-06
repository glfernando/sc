/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

#include <app/shell.h>

import board.init;
import board.debug;
import board.power;
import lib.heap;
import lib.fmt;
import core.cpu;

using sc::lib::fmt::println;

extern void (*__init_array_start[])();
extern void (*__init_array_end[])();

void init_array() {
    for (void (**f)() = __init_array_start; f < __init_array_end; ++f)
        (*f)();
}

extern "C" [[noreturn]] void init() {
    core::cpu::early_init();
    sc::board::early_init();

    sc::lib::heap::init();

    init_array();

    core::cpu::init();
    sc::board::init();

    println("Welcome to SC");

#ifdef RUN_ALL_TESTS
    auto ret = shell_exec_cmd("test all");
    sc::board::poweroff(ret);
#endif

    // just run the shell, there is nothing else to do
    shell_run();

    sc::board::poweroff();
    for (;;) {}
}
