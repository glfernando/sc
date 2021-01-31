/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>
#include <stdio.h>

export module board.init;

export namespace sc::board {

void early_init() {
    //asm volatile("b .");
}

void init() {
}

}  // namespace sc::board
