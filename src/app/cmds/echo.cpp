/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

/*
 * Command to echo a message
 */

#include <app/shell.h>
#include <errcodes.h>
#include <string.h>

import lib.fmt;

using lib::fmt::print;
using lib::fmt::println;

void cmd_echo_usage() {
    println("echo [string ...]");
}

static int cmd_echo(int argc, char const* argv[]) {
    if (argc > 1)
        print("{}", argv[1]);
    for (int i = 2; i < argc; ++i)
        print(" {}", argv[i]);
    println("");

    return 0;
}

shell_declare_static_cmd(echo, "echo same input string", cmd_echo, cmd_echo_usage);
