/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stddef.h>
#include <stdint.h>

export module picoic.ui;

import device.gpio;
import device.i2c;
import board.peripherals;
import lib.ttf;
import std.string;
import lib.fmt;
import lib.gpio;
import lib.ft6206;
import lib.ili9341;
import lib.async;

using lib::fmt::println;

export namespace picoic::ui {

void init() {
    auto& gpio = board::peripherals::default_gpio();
    gpio.set_func(4, soc::rp2040::gpio::func::SPI);
    // gpio.set_func(5, soc::rp2040::gpio::func::SPI);
    gpio.set_func(6, soc::rp2040::gpio::func::SPI);
    gpio.set_func(7, soc::rp2040::gpio::func::SPI);

    gpio.set(5, 1);
    device::gpio::config_t cs_config = {.dir = device::gpio::dir::OUTPUT};
    gpio.config(5, cs_config);
    gpio.set(26, 1);
    gpio.config(26, cs_config);

    device::gpio::config_t i2c_config = {
        .dir = device::gpio::dir::INPUT,
        .pull = device::gpio::pull::UP,
    };
    gpio.config(2, i2c_config);
    gpio.config(3, i2c_config);

    gpio.set_func(2, soc::rp2040::gpio::func::I2C);
    gpio.set_func(3, soc::rp2040::gpio::func::I2C);

    lib::ttf::init();

    lib::ili9341::init();
    lib::ili9341::clear_screen(0x0);
    lib::ili9341::fill_rect(100, 100, 50, 50, 0x07e0);
    lib::ili9341::text(105, 110, 8, 0, 0x07f0, "Press");

    auto i2c = device::manager::find<device::i2c>("i2c1");
    lib::ft6206::init(i2c, 0x38, 27, 240, 320);

    lib::ft6206::register_callback(
        [](uint8_t event, uint16_t x, uint16_t y) {
            // println("got event {}, x={}, y={}", event, x, y);
            if (event == 2) {
                if (x >= 100 && x < 150 && y >= 100 && y < 150) {
                    lib::ili9341::fill_rect(100, 100, 50, 50, ~0x07e0);
                    lib::ili9341::text(105, 110, 8, ~0, ~0x07e0, "Press");
                }
            } else {
                lib::ili9341::fill_rect(100, 100, 50, 50, 0x07e0);
                lib::ili9341::text(105, 110, 8, 0, 0x07e0, "Press");
            }
        },
        0x3);
}

}  // namespace picoic::ui
