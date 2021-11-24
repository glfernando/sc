/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

#include <app/shell.h>

import lib.async;
import lib.fmt;

using lib::async;
using lib::fmt::println;

int main() {
    println("PICO Irrigation Controller");

    // run shell in parallel for debugging
    async ashell{shell_run};
}
