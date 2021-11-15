/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module soc.rp2040.multicore;

#include <stddef.h>
#include <stdint.h>

import lib.reg;
import soc.rp2040.mailbox;

using lib::reg::reg32;

// TODO: make it configurable
constexpr uintptr_t CORE1_STACK_START = 0x20041000;
constexpr size_t CORE1_STACK_SIZE = 0x1000;

namespace {

enum {
    SEQ_RESTART = 0,
    SEQ_START = 1,
};

}

export namespace soc::rp2040::multicore {

void core1_start(void (*entry)()) {
    uintptr_t stack = CORE1_STACK_START + CORE1_STACK_SIZE;
    uintptr_t vtor = reg32(0xe0000000 + 0xed08);

    uintptr_t seq[] = {
        SEQ_RESTART, SEQ_START, vtor, stack, (uintptr_t)entry,
    };

    size_t i = 0;
    do {
        uintptr_t cmd = seq[i];

        if (!cmd) {
            mailbox::flush();
        }

        mailbox::write(cmd);
        uintptr_t res = mailbox::read();
        i = cmd == res ? i + 1 : 0;
    } while (i < (sizeof seq / sizeof seq[0]));
}

}  // namespace soc::rp2040::multicore
