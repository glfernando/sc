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

export namespace board {

void early_init() {}

void init() {
    soc::rp2040::init();
    peripherals::init();
}

}  // namespace board
