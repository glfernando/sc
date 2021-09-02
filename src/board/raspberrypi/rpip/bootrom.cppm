/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>

export module board.pico.bootrom;

import lib.reg;

using lib::reg::reg16;

using rom_table_lookup_t = void* (*)(uint16_t* table, uint32_t code);

consteval uint32_t rom_table_code(char c1, char c2) {
    return static_cast<uint32_t>(c2) << 8 | c1;
}

static void* rom_func_lookup(uint32_t code) {
    auto func = reinterpret_cast<rom_table_lookup_t>(reg16(0x18));
    uint16_t* table = reinterpret_cast<uint16_t*>(reg16(0x14));
    return func(table, code);
}

enum {
    POPCOUNT32,
    REVERSE32,
    CLZ32,
    CTZ32,
    FUNC_MAX,
};

static void* rom_funcs[FUNC_MAX];

export namespace board::pico::bootrom {

extern "C" {

// clang-format off
__attribute__((naked))
int __popcountsi2 (unsigned) {
    asm volatile(R"(
        ldr %0, [%0]
        bx %0
    )" :: "r"(&rom_funcs[POPCOUNT32]) : "r0");
}

__attribute__((naked))
int __clzsi2(unsigned) {
    asm volatile(R"(
        ldr %0, [%0]
        bx %0
    )" :: "r"(&rom_funcs[CLZ32]) : "r0");
}

__attribute__((naked))
int __ctzsi2(unsigned) {
    asm volatile(R"(
        ldr %0, [%0]
        bx %0
    )" :: "r"(&rom_funcs[CTZ32]) : "r0");
}
// clang-format on
}

void init() {
    rom_funcs[POPCOUNT32] = rom_func_lookup(rom_table_code('P', '3'));
    rom_funcs[CLZ32] = rom_func_lookup(rom_table_code('L', '3'));
    rom_funcs[CTZ32] = rom_func_lookup(rom_table_code('T', '3'));
}

}  // namespace board::pico::bootrom
