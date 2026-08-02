/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Fernando Lugo <lugo.fernando@gmail.com>
 */

#include <app/shell.h>
#include <errcodes.h>
#include <stdint.h>
#include <string.h>

import lib.fmt;
import lib.time;
import std.string;

using namespace lib::time;
using lib::fmt::println;
using std::string;

void cmd_time_usage() {
    println("time <command>");
}

static int cmd_time(int argc, char const* argv[]) {
    if (argc < 2) {
        cmd_time_usage();
        return ERR_INVALID_ARGS;
    }

    string cmd_str = "";
    for (int i = 1; i < argc; ++i) {
        if (i != 1) {
            cmd_str += " ";
        }
        cmd_str += argv[i];
    }

    time_ns_t t_start = now();
    int ret = shell_exec_cmd(cmd_str.c_str());
    time_ns_t t_end = now();

    uint64_t diff_ms = (t_end.count() - t_start.count()) / 1000000;
    println("real {}.{:03}s", diff_ms / 1000, diff_ms % 1000);

    return ret;
}

shell_declare_static_cmd(time, "measure command execution time", cmd_time, cmd_time_usage);
