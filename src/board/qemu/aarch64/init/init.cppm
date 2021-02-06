/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module board.init;

import board.debug;

export namespace sc::board {

void init() {
    sc::board::debug::uart::init();
}

}
