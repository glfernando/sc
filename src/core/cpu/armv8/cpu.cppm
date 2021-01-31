/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <errcodes.h>

#include "exception_vector.h"

export module core.cpu.arch;

export import core.cpu.armv8.common;

import core.cpu.armv8.exception;
import lib.fmt;

using sc::lib::fmt::println;

export using cpu_irq_handler = void(*)(int vec, void* data);

namespace core::cpu {

static cpu_irq_handler cpu_handler;
static void* cpu_handler_data;

static int cpu_exception_handler(arvm8::exception::regs*) {
    if (cpu_handler)
        cpu_handler(0, cpu_handler_data);
    else
        println("no cpu irq handler");
    return 0;
}

}  // namespace core::cpu

export namespace core::cpu {

void early_init() {}

void init() {
    arvm8::exception::init();
    register_exception_handler(EXCEPTION_TYPE_SPX_IRQ, cpu_exception_handler);
}

void register_irq_handler(cpu_irq_handler handler, void* data) {
    cpu_handler = handler;
    cpu_handler_data = data;
}

}  // namespace core::cpu
