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

import lib.fmt;
import std.string;
import std.vector;

using lib::fmt::println;
using std::string;
using std::vector;

static void cmd_loop_usage() {
    println("loop <range> <command>");
    // TODO: add support for range with variable name with syntax likeL: loop [var=][x..]y[:step].
    // Examples
    // loop i=0..10:2 echo $i
    // it should print:
    // 0
    // 2
    // 4
    // 6
    // 8
}

static vector<string> split_by(string const& str, char delim) {
    vector<string> strings;

    string tmp = "";
    for (auto c : str) {
        if (c != delim) {
            tmp += c;
        } else if (tmp != "") {
            strings.push_back(tmp);
            tmp = "";
        }
    }

    // push remaining
    if (tmp != "")
        strings.push_back(tmp);

    return strings;
}

static int cmd_loop(int argc, char const* argv[]) {
    if (argc < 3) {
        cmd_loop_usage();
        return ERR_INVALID_ARGS;
    }

    size_t num = strtoul(argv[1], nullptr, 0);

    string cmd_str = "";

    for (int i = 2; i < argc; ++i) {
        if (i != 2)
            cmd_str += " ";
        cmd_str += argv[i];
    }

    auto cmds = split_by(cmd_str, ';');
    for (size_t i = 0; i != num; ++i)
        for (auto& cmd : cmds)
            shell_exec_cmd(cmd.c_str());

    return 0;
}

shell_declare_static_cmd(loop, "loop over a range and execute commands", cmd_loop, cmd_loop_usage);
