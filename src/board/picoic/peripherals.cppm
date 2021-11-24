/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdio.h>

export module board.peripherals;

export import device.uart.pl011;
export import device.console.uart;
export import device.timer.rp2040;
export import device.intc.nvic;
export import soc.rp2040.gpio;
export import soc.rp2040.i2c;
export import soc.rp2040.spi;

import soc.rp2040.address_map;
import std.string;
import lib.fmt;
import lib.exception;
import soc.rp2040.mailbox;

using lib::exception;
using namespace soc::rp2040::address_map;

static constexpr device::pl011::platform_data uart0_pdata{
    .base = UART0_BASE,
    .freq = 125'000'000,
    .baudrate = 230400,
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

static constexpr soc::rp2040::i2c::platform_data i2c0_pdata{
    .base = I2C0_BASE,
    .freq = 125'000'000,
    .mode = device::i2c::mode::FAST,
};

static constexpr soc::rp2040::i2c::platform_data i2c1_pdata{
    .base = I2C1_BASE,
    .freq = 125'000'000,
    .mode = device::i2c::mode::FAST,
};

static constexpr soc::rp2040::spi::platform_data spi0_pdata{
    .base = SPI0_BASE,
    .freq = 125'000'000,
    .baudrate = 10'000'000,
};

static constexpr soc::rp2040::spi::platform_data spi1_pdata{
    .base = SPI1_BASE,
    .freq = 125'000'000,
    .baudrate = 10'000'000,
};

static device::pl011 uart0("uart0", uart0_pdata);

static device::uart_console con0("con0", uart0);

static device::timer_rp2040 timer0("timer0", timer0_pdata);

static device::nvic nvic("nvic", nvic_pdata);

static soc::rp2040::gpio gpio("gpio", 16 + 13);

static soc::rp2040::i2c i2c0("i2c0", i2c0_pdata);
static soc::rp2040::i2c i2c1("i2c1", i2c1_pdata);

static soc::rp2040::spi spi0("spi0", spi0_pdata);
static soc::rp2040::spi spi1("spi1", spi1_pdata);

export namespace board::peripherals {

void init() {
    nvic.init();
    // register GIC
    device::manager::register_device(&nvic);

    uart0.init();
    lib::fmt::register_console(&con0);
    printf_set_putchar_func([](int c) {
        uart0.putc(c);
        return c;
    });

    timer0.init();
    device::manager::register_device(&timer0);

    gpio.init();
    device::manager::register_device(&gpio);

    i2c0.init();
    i2c1.init();
    device::manager::register_device(&i2c0);
    device::manager::register_device(&i2c1);

    device::manager::register_device(&spi0);
    device::manager::register_device(&spi1);
}

void init_sec() {
    nvic.init();
    soc::rp2040::mailbox::init();
    nvic.request_irq(
        16 + 16, device::intc::FLAG_START_ENABLED,
        [](unsigned, void*) { soc::rp2040::mailbox::flush(); }, nullptr);
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
