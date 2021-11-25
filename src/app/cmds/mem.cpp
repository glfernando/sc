/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

/*
 * Command to read/write or dump memory regions. It has options to print offset and ascii code
 * during memory dump. Later, when multicore is enabled it should should have have an option to
 * specify the core that will do the operation
 */

#include <app/shell.h>
#include <errcodes.h>
#include <stdint.h>
#include <string.h>

import lib.fmt;
import lib.hexdump;
import std.string;
import core.thread;
import lib.cpu;
import lib.async;

using lib::async;
using lib::hexdump;
using lib::fmt::println;
using std::string;

void cmd_mem_usage() {
    println(
        "mem [options] <[0x]hex_addr>[:size|..<[0x]hex_range> [value]\n"
        "options:\n"
        "   -c <num>    cpu core used to read/write memory\n"
        "   -o          print offset during memory dump\n"
        "   -a          print ascii code during memory dump\n"
        "   -g <num>    group by <num> during memory dump\n"
        "Note:\n"
        "mem command enter memory dump mode when size is different from 1/2/4/8");
}

static int mem_task(bool write, unsigned long val, size_t size, uintptr_t addr, size_t group,
                    bool offset, bool ascii) {
    if (write) {
        switch (size) {
        case 1:
            *reinterpret_cast<uint8_t*>(addr) = val;
            break;
        case 2:
            *reinterpret_cast<uint16_t*>(addr) = val;
            break;
        case 4:
            *reinterpret_cast<uint32_t*>(addr) = val;
            break;
        case 8:
            *reinterpret_cast<uint64_t*>(addr) = val;
            break;
        default:
            println("unsupported size for writing, use 1/2/4/8");
            return ERR_INVALID_ARGS;
        }
        return 0;
    }

    // read
    switch (size) {
    case 1:
        println("{:02x}", *reinterpret_cast<uint8_t*>(addr));
        break;
    case 2:
        println("{:04x}", *reinterpret_cast<uint16_t*>(addr));
        break;
    case 4:
        println("{:08x}", *reinterpret_cast<uint32_t*>(addr));
        break;
    case 8:
        println("{:016x}", *reinterpret_cast<uint64_t*>(addr));
        break;
    default:
        // use hexdumo for any other size
        hexdump(reinterpret_cast<void*>(addr), size, group, offset, ascii);
    }

    return 0;
}

constexpr unsigned CPU_ANY = ~0U;

static int cmd_mem(int argc, char const* argv[]) {
    bool offset = false;
    bool ascii = false;
    size_t group = 4;
    int pos_argc = 0;
    unsigned cpu = CPU_ANY;
    for (int i = 0; i < argc; ++i) {
        auto arg = argv[i];

        if (*arg++ != '-') {
            // positional argument
            argv[pos_argc++] = argv[i];
            continue;
        }

        switch (*arg) {
        case 'c':
            if (++i == argc) {
                println("missing cor number");
                cmd_mem_usage();
                return ERR_INVALID_ARGS;
            }
            cpu = strtoul(argv[i], NULL, 0);
            if (cpu >= core::thread::core_num) {
                println("invalid core number {}", cpu);
                return ERR_INVALID_ARGS;
            }
            break;
        case 'o':
            offset = true;
            break;
        case 'a':
            ascii = true;
            break;
        case 'g':
            if (++i == argc) {
                println("missing group number");
                cmd_mem_usage();
                return ERR_INVALID_ARGS;
            }
            group = strtoul(argv[i], NULL, 0);
            if (!group || ((group - 1) & group) || group > 8) {
                println("invalid group number {}", group);
                return ERR_INVALID_ARGS;
            }
            break;
        default:
            println("invalid option {:c}", *arg);
            cmd_mem_usage();
            return ERR_INVALID_ARGS;
        }
    }
    // update new argument number
    argc = pos_argc;

    if (argc < 2) {
        cmd_mem_usage();
        return ERR_INVALID_ARGS;
    }

    auto ptr = argv[1];
    char* endptr;

    uintptr_t addr = strtoul(ptr, &endptr, 16);
    if (ptr == endptr) {
        println("invalid address value");
        return ERR_INVALID_ARGS;
    }

    size_t size = 4;
    if (*endptr == ':') {
        ptr = endptr + 1;
        size = strtoul(ptr, &endptr, 0);
        if (ptr == endptr || *endptr) {
            println("invalid size argument");
            return ERR_INVALID_ARGS;
        }
    } else if (*endptr == '.' && *(endptr + 1) == '.') {
        // range value
        ptr = endptr + 2;
        uintptr_t end = strtoul(ptr, &endptr, 16);
        if (ptr == endptr) {
            println("invalid end address of range");
            return ERR_INVALID_ARGS;
        }
        if (end <= addr) {
            println("end address needs to be bigger than start address");
            return ERR_INVALID_ARGS;
        }
        size = end - addr;
    } else if (*endptr) {
        println("invalid  address value");
        return ERR_INVALID_ARGS;
    }

    unsigned long val = 0;
    bool write = false;
    if (argc == 3) {
        // we are wrting a value
        ptr = argv[2];
        val = strtoul(ptr, &endptr, 0);
        if (ptr == endptr) {
            println("invalid write value");
            return ERR_INVALID_ARGS;
        }
        write = true;
    }

    // avoid spawing a new thread if not needed
    if (cpu == CPU_ANY || cpu == lib::cpu::id()) {
        return mem_task(write, val, size, addr, group, offset, ascii);
    }

    // spawn a new thread to read/write memory
    async a{"mem-task", 1U << cpu, mem_task, write, val, size, addr, group, offset, ascii};
    return a.wait_for_result();
}

shell_declare_static_cmd(mem, "read or write memory", cmd_mem, cmd_mem_usage);
