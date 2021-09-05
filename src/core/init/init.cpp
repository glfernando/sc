/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

#include <app/shell.h>

import board.init;
import board.power;
import lib.heap;
import lib.fmt;
import core.cpu;
import core.thread;

using lib::fmt::println;

extern void (*__init_array_start[])();
extern void (*__init_array_end[])();
extern int main();

void init_array() {
    for (void (**f)() = __init_array_start; f < __init_array_end; ++f)
        (*f)();
}

// allow to override main
__attribute__((weak)) int main() {
    shell_run();
    return 0;
}

extern "C" [[noreturn]] void init() {
    core::cpu::early_init();
    board::early_init();

    lib::heap::init();

    init_array();

    core::cpu::init();
    board::init();

    // init thread framework, after that we will be running in the main thread
    core::thread::init();

    println("Welcome to SC");

#ifdef RUN_ALL_TESTS
    auto ret = shell_exec_cmd("test all");
    board::poweroff(ret);
#endif

    main();

    board::poweroff();
    for (;;) {}
}
