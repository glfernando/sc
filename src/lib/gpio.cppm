/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <app/shell.h>
#include <errcodes.h>
#include <string.h>

export module lib.gpio;

export import device.gpio;
import board.peripherals;
import lib.fmt;
import std.string;

using lib::fmt::println;
using std::string;

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

}  // namespace lib::gpio

void cmd_gpio_usage() {
    println("gpio <num> [val]");
    println("gpio <num> config <flag1> [flags2 ... flagsN]");
    println("flags: input, output, high, low, rising, falling, both, pullup, pulldown");
}

static int cmd_gpio(int argc, char const* argv[]) {
    if (argc < 2) {
        cmd_gpio_usage();
        return 0;
    }

    using namespace lib;

    auto ptr = argv[1];
    char* endptr;
    unsigned gpio = strtoul(ptr, &endptr, 0);
    if (ptr == endptr || *endptr) {
        println("invalid gpio number");
        return ERR_INVALID_ARGS;
    }

    if (argc >= 3 && string("config") == argv[2]) {
        // config
        if (argc < 4) {
            println("no flags passed\n");
            return ERR_INVALID_ARGS;
        }
        gpio::config_t config{};

        for (int i = 3; i < argc; ++i) {
            string flag = argv[i];

            if (flag == "input")
                config.dir = gpio::dir::INPUT;
            else if (flag == "output")
                config.dir = gpio::dir::OUTPUT;
            else if (flag == "high")
                config.trigger = gpio::trigger::HIGH;
            else if (flag == "low")
                config.trigger = gpio::trigger::LOW;
            else if (flag == "rising")
                config.trigger = gpio::trigger::RISING;
            else if (flag == "falling")
                config.trigger = gpio::trigger::FALLING;
            else if (flag == "both")
                config.trigger = gpio::trigger::BOTH;
            else if (flag == "pullup")
                config.pull = gpio::pull::UP;
            else if (flag == "pulldown")
                config.pull = gpio::pull::DOWN;
            else {
                println("invalid flag {}", flag);
                return ERR_INVALID_ARGS;
            }

            gpio::config(gpio, config);
        }

    } else if (argc == 2) {
        // read
        println("{}", gpio::get(gpio));
    } else {
        // write
        string val = argv[2];
        if (val == "0") {
            gpio::set(gpio, false);
        } else if (val == "1") {
            gpio::set(gpio, true);
        } else {
            println("invalid gpio value");
        }
    }

    return 0;
}

shell_declare_static_cmd(gpio, "modify gpio state", cmd_gpio, cmd_gpio_usage);
