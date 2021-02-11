/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>

export module core.cpu.armv6m.exception;

namespace core::cpu::armv6m::exception {

constexpr unsigned EXT_INT_MAX = 32;

using handler_t = void (*)(void);

struct vector_table_t {
    void* sp_main;
    handler_t reset;
    handler_t nmi;
    handler_t hard_fault;
    handler_t mem_manage;
    handler_t bus_fault;
    handler_t usage_fault;
    handler_t reserved[4];
    handler_t svc;
    handler_t debug_monitor;
    handler_t reserved1;
    handler_t pend_sv;
    handler_t systick;
    handler_t ext_int[EXT_INT_MAX];
};

export extern "C" uint8_t __stack_end[];
export extern "C" void _start(void);

void nmi_handler() {
    asm volatile("b .");
}

void hard_fault_handler() {
    asm volatile("b .");
}

export const vector_table_t vector_table __attribute__((section(".vector_table")))
__attribute__((used)) = {
    .sp_main = __stack_end,
    .reset = _start,
    .nmi = nmi_handler,
    .hard_fault = hard_fault_handler,
};

}  // namespace core::cpu::armv6m::exception

export namespace core::cpu::armv6m::exception {

void init() {}

}  // namespace core::cpu::armv6m::exception
