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
import board.pico.bootrom;

export namespace board {

void early_init() {
    board::pico::bootrom::init();
}

void init() {
    soc::rp2040::init();
    peripherals::init();
}

void late_init() {
}

}  // namespace board
