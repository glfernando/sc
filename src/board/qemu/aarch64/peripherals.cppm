/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module board.peripherals;
export import device.uart.pl011;
export import device.console.uart;

import std.string;

static constexpr device::pl011::platform_data uart0_pdata{
    .base = 0x0900'0000,
    .freq = 24'000'000,
    .baudrate = 115200,
};

static device::pl011 uart0("uart0", uart0_pdata);

static device::uart_console con0("con0", uart0);

export namespace sc::board::peripherals {

void init() {
    uart0.init();
}

auto& default_console() {
    return con0;
}

}  // namespace sc::board::peripherals
