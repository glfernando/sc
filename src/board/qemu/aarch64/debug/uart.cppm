/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <common.h>

export module board.debug.uart;

import lib.reg;

using namespace sc::lib::reg;

enum reg_offset : unsigned {
    UART_DR	= 0x00,
    UART_FR	= 0x18,
    UART_LCR_H	= 0x2c,
    UART_CR	= 0x30,
};

constexpr uintptr_t uart_base = CONFIG_DEBUG_UART_BASE;

static inline volatile uint32_t& reg(reg_offset offset) {
    return reg32(uart_base + offset);
}

export namespace sc::board::debug::uart {

void init()
{
    reg(UART_LCR_H) = 1 << 4;
    reg(UART_CR) = 0x301;
}

int putchar(int c)
{
    reg(UART_DR) = c;
    if (c == '\n')
	putchar('\r');
    return c;
}

void puts(char const* str) {
    while (int c = *str++)
	putchar(c);
}

}
