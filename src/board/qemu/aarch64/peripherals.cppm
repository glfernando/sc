/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module board.peripherals;
export import device.uart.pl011;
export import device.intc.gic;
export import device.console.uart;
export import device.console.uart;
export import device.timer.arm;

import std.string;
import device;
import lib.fmt;

static constexpr device::pl011::platform_data uart0_pdata{
    .base = 0x0900'0000,
    .freq = 24'000'000,
    .baudrate = 115200,
    .irq = 33,
};

static device::pl011 uart0("uart0", uart0_pdata);

static device::uart_console con0("con0", uart0);

static constexpr device::gic::platform_data gicv2_pdata{
    .dbase = 0x0800'0000,
    .cbase = 0x0801'0000,
};
static device::gic gicv2("gic", gicv2_pdata);

static device::timer_arm::platform_data timer_pdata{
    .irq = 30,
};

static device::timer_arm timer0("timer0", timer_pdata);

export namespace board::peripherals {

void init() {
    gicv2.init();
    // register GIC
    device::manager::register_device(&gicv2);

    uart0.init();
    lib::fmt::register_console(&con0);

    timer0.init();
    device::manager::register_device(&timer0);
}

auto& default_console() {
    return con0;
}

device::gic& default_intc() {
    return gicv2;
}

auto& default_timer() {
    return timer0;
}

}  // namespace board::peripherals
