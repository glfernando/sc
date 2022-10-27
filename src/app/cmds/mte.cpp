/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2022 Fernando Lugo <lugo.fernando@gmail.com>
 */

/*
 * Command to sleep for speficied time
 */

#include <app/shell.h>
#include <errcodes.h>
#include <string.h>

import core.thread;
import lib.fmt;
import std.string;
import arch.aarch64.mte;

using lib::fmt::println;
using std::string;

void cmd_mte_usage() {
    println("mte irg <addr>");
    println("mte tag <addr> [size]");
    println("mte load <addr>");
    println("mte teststack");
}

static int cmd_mte(int argc, char const* argv[]) {
    println("argc {}, argv {}", argc, argv);
    if (argc < 2) {
        cmd_mte_usage();
        return ERR_INVALID_ARGS;
    }

    string cmd = argv[1];

    if (argc == 3 && cmd == "irg") {
        auto addr = strtoul(argv[2], NULL, 16);

        unsigned long tag = aarch64::irg(addr);
        println("addr = {:#x}, tag = {:#x}", addr, tag);
    } else if (argc >= 3 && cmd == "tag") {
        auto addr = strtoul(argv[2], NULL, 16);
        size_t size = 16;
        if (argc >= 4) {
            size = strtoul(argv[3], NULL, 0);
        }
        println("tagging region {:#x} size {:#x}", addr, size);

        for (size_t i = 0; i < size; i += 16) {
            aarch64::stg(addr);
            addr += 16;
        }

        asm volatile("dsb sy");
        asm volatile("isb");
    } else if (argc == 3 && cmd == "load") {
        auto addr = strtoul(argv[2], NULL, 16);
        unsigned long tag = aarch64::ldg(addr);
        println("addr = {:#x}, tag = {:#x}", addr, tag);
    } else if (argc == 2 && cmd == "teststack") {
        int a = 10;
        int b = 15;
        println("a addr {:p} b addr {:p} a*b={}", &a, &b, a * b);
    } else {
        cmd_mte_usage();
    }

    return 0;
}

shell_declare_static_cmd(mte, "mte command", cmd_mte, cmd_mte_usage);
