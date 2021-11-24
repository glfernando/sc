/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>
#include <stdio.h>

export module board.init;

import soc.rp2040;
import board.peripherals;
import soc.rp2040.multicore;
import core.thread;
import lib.reg;
import soc.rp2040.mailbox;

using namespace soc::rp2040;

static void sec_entry() {
    board::peripherals::init_sec();
    core::thread::init();
}

static void cpu1_start() {
    multicore::core1_start(sec_entry);

    auto& nvic = board::peripherals::default_intc();
    nvic.request_irq(
        16 + 15, device::intc::FLAG_START_ENABLED,
        [](unsigned, void*) { soc::rp2040::mailbox::flush(); }, nullptr);
}

export namespace board {

void early_init() {
    soc::rp2040::early_init();
}

void init() {
    soc::rp2040::init();
    peripherals::init();
}

void late_init() {
    cpu1_start();
}

}  // namespace board
