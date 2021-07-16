/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module board.peripherals;

export import device.uart.pl011;
export import device.console.uart;
export import device.timer.rp2040;
export import device.intc.nvic;
export import soc.rp2040.gpio;

import soc.rp2040.address_map;
import std.string;
import lib.fmt;

using namespace soc::rp2040::address_map;

static constexpr device::pl011::platform_data uart0_pdata{
    .base = UART0_BASE,
    .freq = 125'000'000,
    .baudrate = 115200,
    .irq = 16 + 20,
};

static constexpr device::nvic::platform_data nvic_pdata{
    .base = PPB_BASE,
    .irq_num = 16 + 32,
};

static constexpr device::timer_rp2040::platform_data timer0_pdata{
    .base = TIMER_BASE,
    .irq = 16,
};

static device::pl011 uart0("uart0", uart0_pdata);

static device::uart_console con0("con0", uart0);

static device::timer_rp2040 timer0("timer0", timer0_pdata);

static device::nvic nvic("nvic", nvic_pdata);

static soc::rp2040::gpio gpio("gpio", 16 + 13);

export namespace board::peripherals {

void init() {
    nvic.init();
    // register GIC
    device::manager::register_device(&nvic);

    uart0.init();
    lib::fmt::register_console(&con0);

    timer0.init();
    device::manager::register_device(&timer0);

    gpio.init();
}

auto& default_console() {
    return con0;
}

auto& default_timer() {
    return timer0;
}

auto& default_intc() {
    return nvic;
}

auto& default_gpio() {
    return gpio;
}

}  // namespace board::peripherals
