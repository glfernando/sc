/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module board.init;

import soc.rp2350;
import board.peripherals;

export namespace board {

void early_init() {
    soc::rp2350::early_init();
}

void init() {
    soc::rp2350::init();
    peripherals::init();
}

void late_init() {}

}  // namespace board
