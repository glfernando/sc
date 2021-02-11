/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module board.peripherals;

export import device.uart.pl011;
export import device.console.uart;
export import device.timer.rp2040;

import std.string;
import lib.fmt;

static constexpr device::pl011::platform_data uart0_pdata{
    .base = 0x4003'4000,
    .freq = 125'000'000,
    .baudrate = 115200,
};

static device::pl011 uart0("uart0", uart0_pdata);

static device::uart_console con0("con0", uart0);

static device::timer_rp2040 timer0("timer0");

export namespace sc::board::peripherals {

void init() {
    uart0.init();
    sc::lib::fmt::register_console(&con0);
}

auto& default_console() {
    return con0;
}

auto& default_timer() {
    return timer0;
}

}  // namespace sc::board::peripherals
