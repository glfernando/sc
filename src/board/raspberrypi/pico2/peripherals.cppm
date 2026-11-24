/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdio.h>

export module board.peripherals;

export import device.uart.pl011;
export import device.console.uart;
export import device.timer.rp2350;
export import device.intc.nvic;

import soc.rp2350.address_map;
import lib.fmt;

using namespace soc::rp2350::address_map;

static constexpr device::pl011::platform_data uart0_pdata{
    .base = UART0_BASE,
    .freq = 150'000'000,
    .baudrate = 115200,
    .irq = 16 + 33,
};

static constexpr device::nvic::platform_data nvic_pdata{
    .base = PPB_BASE,
    .irq_num = 16 + 52,
};

static constexpr device::timer_rp2350::platform_data timer0_pdata{
    .base = TIMER0_BASE,
    .irq = 16 + 0,
};

device::pl011 uart0("uart0", uart0_pdata);
device::uart_console con0("con0", uart0);
device::timer_rp2350 timer0("timer0", timer0_pdata);
device::nvic nvic("nvic", nvic_pdata);

export namespace board::peripherals {

void init() {
    nvic.init();
    device::manager::register_device(&nvic);

    uart0.init();
    lib::fmt::register_console(&con0);
    printf_set_putchar_func([](int c) {
        uart0.putc(c);
        return c;
    });

    timer0.init();
    device::manager::register_device(&timer0);
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

}  // namespace board::peripherals
