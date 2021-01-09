/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module core.cpu.armv8;

import core.cpu.armv8.exception;

export namespace core::cpu {

void early_init() {}

void init() { arvm8::exception::init(); }

}  // namespace core::cpu
