/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <app/shell.h>
#include <errcodes.h>
#include <stdint.h>
#include <string.h>

export module lib.i2c;

import device.i2c;
import lib.fmt;
import std.string;
import std.vector;

using lib::fmt::println;
using std::string;
using std::vector;

void cmd_i2c_usage() {
    println("i2c list");
    println("i2c read <name> <addr> [size]");
    println("i2c write <name> <addr> <byte1> [byte2 .. byteN]");
    println("i2c reg <name> <addr> <reg> [data]");
}

static void list_i2c_buses() {
    auto devices = device::manager::find_all<device::i2c>();

    for (auto dev : devices)
        println("{}", dev->name());
}

static device::i2c* get_device(string const& name) {
    auto devices = device::manager::find_all<device::i2c>();

    for (auto dev : devices)
        if (name == dev->name())
            return dev;
    return nullptr;
}

static int cmd_i2c(int argc, char const* argv[]) {
    if (argc < 2) {
        cmd_i2c_usage();
        return 0;
    }

    string cmd = argv[1];

    if (cmd == "list") {
        list_i2c_buses();
        return 0;
    }

    if (argc < 4) {
        cmd_i2c_usage();
        return 0;
    }

    auto i2c = get_device(argv[2]);
    if (i2c == nullptr) {
        println("could not find device {}", argv[2]);
        return 0;
    }

    uint8_t addr = strtoul(argv[3], NULL, 0);

    if (cmd == "read") {
        size_t size = argc == 5 ? strtoul(argv[4], NULL, 0) : 1;
        vector<uint8_t> buf;
        buf.resize(size);
        i2c->read(addr, buf.data(), buf.size());
        for (auto v : buf)
            println("{:#x}", v);
    } else if (argc >= 5 && cmd == "write") {
        vector<uint8_t> buf;
        for (int i = 4; i < argc; ++i) {
            uint8_t val = strtoul(argv[i], NULL, 0);
            buf.push_back(val);
        }
        i2c->write(addr, buf.data(), buf.size());
    } else if ((argc == 5 || argc == 6) && cmd == "reg") {
        if (argc == 5) {
            uint8_t reg = strtoul(argv[4], NULL, 0);
            uint8_t val = i2c->reg(addr, reg);
            println("{:#x}", val);
        } else {
            uint8_t reg = strtoul(argv[4], NULL, 0);
            uint8_t val = strtoul(argv[5], NULL, 0);
            println("writing {:#x} to {:#x}", val, reg);
            i2c->reg(addr, reg, val);
        }
    } else {
        cmd_i2c_usage();
    }

    return 0;
}

shell_declare_static_cmd(i2c, "read/write from i2c bus", cmd_i2c, cmd_i2c_usage);
