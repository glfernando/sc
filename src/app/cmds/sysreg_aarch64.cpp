/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

/*
 * Command to read and write system register for AARCH64
 */

#include <app/shell.h>
#include <arch/aarch64/sysreg.h>
#include <errcodes.h>
#include <string.h>

import lib.fmt;
import std.string;

using lib::fmt::println;
using lib::fmt::sprint;
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

//TODO: add helper macro for adding system register for all ELX

sysreg_data sysregs[] = {
    SYSREG(cntfrq_el0),
    SYSREG(cntp_ctl_el0),
    SYSREG(cntp_cval_el0),
    SYSREG(cntp_tval_el0),
    SYSREG_RO(cntpct_el0),
    SYSREG_RO(currentel),
    SYSREG(daif),
#ifdef CONFIG_AARCH64_MTE
    SYSREG(gcr_el1),
#endif
    SYSREG(mair_el1),
    SYSREG_RO(mpidr_el1),
#ifdef CONFIG_AARCH64_MTE
    SYSREG(rgsr_el1),
#endif
    SYSREG(sctlr_el1),
#ifdef CONFIG_AARCH64_MTE
    SYSREG_RO(tco),
#endif
    SYSREG(tcr_el1),
    SYSREG(ttbr0_el1),
    SYSREG(ttbr1_el1),
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
        // let's use current EL to guess the register name
        auto el = sysreg_read(currentel) >> 2;
        if (el > 0 && el < 4) {
            string name = sprint("{}_el{}", argv[1], el);
            data = find_sysreg(name.c_str());
        }
        if (!data) {
            println("system register ({}) not found", argv[1]);
            return ERR_NOT_FOUND;
        }
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
