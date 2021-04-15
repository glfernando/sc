/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

#include <app/shell.h>

import lib.time;
import lib.timer;
import lib.gpio;

using lib::timer;
using namespace lib::time;
using namespace lib;

constexpr unsigned LED = 25;

int main() {
    gpio::config_t led_config = {.dir = gpio::dir::OUTPUT};

    gpio::config(LED, led_config);

    timer t;
    t.start(
        [val = true]() mutable {
            gpio::set(LED, val);
            val = !val;
        },
        1s);

    for (;;) {
        // TODO: change to lib::cpu::idle() when available
        asm volatile("wfi");
    }

    return 0;
}
