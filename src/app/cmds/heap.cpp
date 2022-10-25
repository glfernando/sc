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

import lib.fmt;
import lib.heap;
import std.string;

using lib::fmt::println;
using lib::heap::alloc;
using lib::heap::free;
using std::string;

void cmd_heap_usage() {
    println("heap alloc <size> [alignment]");
    println("heap free <addr>");
}

static int cmd_heap(int argc, char const* argv[]) {
    if (argc < 2) {
        cmd_heap_usage();
        return ERR_INVALID_ARGS;
    }

    string cmd = argv[1];

    if (argc >= 3 && cmd == "alloc") {
        size_t size = strtoul(argv[2], NULL, 0);

        void* ptr;
        if (argc == 3) {
            ptr = alloc(size);
        } else {
            size_t align = strtoul(argv[3], NULL, 0);
            ptr = alloc(size, align);
        }
        println("{:p}", ptr);
    } else if (argc == 3 && cmd == "free") {
        auto ptr = (void*)strtoul(argv[2], NULL, 16);
        free(ptr);
    } else {
        cmd_heap_usage();
    }

    return 0;
}

shell_declare_static_cmd(heap, "heap command", cmd_heap, cmd_heap_usage);
