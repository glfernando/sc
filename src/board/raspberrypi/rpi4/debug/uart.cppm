/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>

export module board.debug.uart;

import lib.reg;

using namespace lib::reg;

enum reg_offset : unsigned {
    UART_DR = 0x00,
    UART_FR = 0x18,
    UART_IBRD = 0x24,
    UART_FBRD = 0x28,
    UART_LCR_H = 0x2c,
    UART_CR = 0x30,
};

constexpr uintptr_t uart_base = CONFIG_DEBUG_UART_BASE;

static inline volatile uint32_t& reg(reg_offset offset) {
    return reg32(uart_base + offset);
}

export namespace board::debug::uart {

void init() {
    // init 921600 8N1
    reg(UART_IBRD) = 3;
    reg(UART_FBRD) = 16;
    reg(UART_LCR_H) = 3 << 5;
    reg(UART_CR) = 0x301;
}

int putchar(int c) {
    reg(UART_DR) = c;
    return c;
}

void puts(char const* str) {
    while (int c = *str++)
        putchar(c);
}

}  // namespace board::debug::uart
