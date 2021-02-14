/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

import lib.fmt;
import lib.time;

using sc::lib::fmt::println;
using namespace sc::lib::time;

int main() {

    for (;;) {
        println("hello world");
        delay(1s);
    }

    return 0;
}
