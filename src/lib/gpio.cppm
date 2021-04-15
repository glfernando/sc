/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module lib.gpio;

export import device.gpio;
import board.peripherals;

export namespace lib::gpio {

// export types as if they belong to the gpio library

using config_t = device::gpio::config_t;
using dir = device::gpio::dir;
using trigger = device::gpio::trigger;
using pull = device::gpio::pull;

void config(unsigned gpio, config_t config) {
    auto& dev = board::peripherals::default_gpio();
    dev.config(gpio, config);
}

void set(unsigned gpio, bool val) {
    auto& dev = board::peripherals::default_gpio();
    dev.set(gpio, val);
}

bool get(unsigned gpio) {
    auto& dev = board::peripherals::default_gpio();
    return dev.get(gpio);
}

};  // namespace lib::gpio
