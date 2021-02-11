/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>
#include <string.h>

export module core.cpu.arch;

import core.cpu.armv6m.exception;

export using cpu_irq_handler = void(*)(int vec, void* data);

export namespace core::cpu {

void early_init() {}

void init() {
    armv6m::exception::init();
}

}  // namespace core::cpu
