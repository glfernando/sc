/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

/*
 * Command to sleep for speficied time
 */

#include <app/shell.h>
#include <errcodes.h>
#include <string.h>

import core.thread;
import lib.fmt;
import lib.time;

using namespace lib::time;
using lib::time::time_ms_t;

using lib::fmt::println;

void cmd_sleep_usage() {
    println("sleep <milliseconds>");
}

static int cmd_sleep(int argc, char const* argv[]) {
    if (argc != 2) {
        cmd_sleep_usage();
        return ERR_INVALID_ARGS;
    }

    time_ms_t period = strtoul(argv[1], nullptr, 10);
    core::thread::sleep(period);

    return 0;
}

shell_declare_static_cmd(sleep, "sleep for specified miliseconds", cmd_sleep, cmd_sleep_usage);
