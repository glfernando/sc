/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

/*
 * Command to read and write system register for ARM
 */

#include <app/shell.h>
#include <arch/arm/sysreg.h>
#include <errcodes.h>
#include <string.h>

import lib.fmt;
import std.string;

using sc::lib::fmt::println;
using sc::lib::fmt::sprint;
using std::string;

struct sysreg_data {
    const char* name;
    unsigned long (*read)();
    void (*write)(unsigned long val);
};

// clang-format off

#define SYSREG_RO(_n)   \
    { #_n, [] { return sysreg_read(_n); }, nullptr}

#define SYSREG_WO(_n)   \
    { #_n, nullptr, [](unsigned long val) { return sysreg_write(_n, val); }}

#define SYSREG(_n)      \
    { #_n, [] { return sysreg_read(_n); }, [](unsigned long val) { return sysreg_write(_n, val); }}

sysreg_data sysregs[] = {
    SYSREG(xpsr),
    SYSREG(msp),
    SYSREG(psp),
    SYSREG(primask),
    SYSREG(control),
};

// clang-format on

void cmd_sysreg_usage() {
    println("sysreg [list] <name> [value]");
}

const sysreg_data* find_sysreg(char const* name) {
    for (auto& data : sysregs)
        if (!strcmp(name, data.name))
            return &data;
    return nullptr;
}

static int cmd_sysreg(int argc, char const* argv[]) {
    if (argc < 2 || argc > 3) {
        cmd_sysreg_usage();
        return ERR_INVALID_ARGS;
    }

    if (!strcmp(argv[1], "list")) {
        for (auto& data : sysregs)
            println("{}", data.name);
        return 0;
    }

    auto data = find_sysreg(argv[1]);
    if (!data) {
        println("system register ({}) not found", argv[1]);
        return ERR_NOT_FOUND;
    }

    if (argc > 2) {
        if (data->write) {
            unsigned long v = strtoul(argv[2], NULL, 0);
            println("writing {:#x} to {}", v, data->name);
            data->write(v);
        } else {
            println("error: read only register");
        }
    } else {
        if (data->read)
            println("{} = {:#x}", data->name, data->read());
        else
            println("error: write only register");
    }

    return 0;
}

shell_declare_static_cmd(sysreg, "read or write system register", cmd_sysreg, cmd_sysreg_usage);
