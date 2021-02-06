/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module board.peripherals;
export import device.uart.pl011;

import std.string;
import lib.fmt;

using sc::lib::fmt::println;

static constexpr device::pl011::platform_data uart0_pdata{
    .base = 0x0900'0000,
    .freq = 24'000'000,
    .baudrate = 115200,
};

static device::pl011 uart0("uart0", uart0_pdata);
bool uart_init = false;

export namespace sc::board::peripherals {

void init() { uart0.init(); }

device::pl011& default_console() { return uart0; }

}  // namespace sc::board::peripherals
