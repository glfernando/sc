/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module core.cpu.arch;

export using cpu_irq_handler = void(*)(int vec, void* data);

export namespace core::cpu {

void early_init() {
}

void init() {
}

void register_irq_handler(cpu_irq_handler handler, void* data) {
}

extern "C" void _start() {
    asm volatile ("b .");
}

}

