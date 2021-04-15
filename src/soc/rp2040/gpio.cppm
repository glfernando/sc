/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>

export module soc.rp2040.gpio;

export import device.gpio;

import std.string;
import lib.fmt;
import soc.rp2040.address_map;
import lib.reg;
import lib.exception;

using lib::exception;
using lib::reg::reg32;
using namespace soc::rp2040::address_map;

namespace {

// clang-format off
enum reg_offset : uint32_t {
    GPIO_IN         = 0x04,
    GPIO_OUT        = 0x10,
    GPIO_OUT_SET    = 0x14,
    GPIO_OUT_CLR    = 0x18,
    GPIO_OE_SET     = 0x24,
    GPIO_OE_CLR     = 0x28,
};

enum drive_strength {
    DRIVE_2mA   = 0,
    DRIVE_4mA   = 1,
    DRIVE_8mA   = 2,
    DRIVE_12mA  = 3,
};

enum gpio_function {
    GPIO_FUNC_XIP   = 0,
    GPIO_FUNC_SPI   = 1,
    GPIO_FUNC_UART  = 2,
    GPIO_FUNC_I2C   = 3,
    GPIO_FUNC_PWM   = 4,
    GPIO_FUNC_SIO   = 5,
    GPIO_FUNC_PIO0  = 6,
    GPIO_FUNC_PIO1  = 7,
    GPIO_FUNC_GPCK  = 8,
    GPIO_FUNC_USB   = 9,
    GPIO_FUNC_NULL = 0x1f,
};
// clang-format on

struct gpio_pad {
    uint32_t slewfast : 1;
    uint32_t schmitt : 1;
    uint32_t pde : 1;
    uint32_t pue : 1;
    uint32_t drive : 2;
    uint32_t ie : 1;
    uint32_t od : 1;
    uint32_t : 24;
};

struct io_ctrl {
    uint32_t funcsel : 5;
    uint32_t : 3;
    uint32_t outover : 2;
    uint32_t : 2;
    uint32_t oeover : 2;
    uint32_t : 2;
    uint32_t inover : 2;
    uint32_t : 10;
    uint32_t irqoever : 2;
    uint32_t : 2;
};

volatile uint32_t& reg(uint32_t offset) {
    return reg32(SIO_BASE + offset);
}

gpio_pad& get_pad(unsigned gpio) {
    return *reinterpret_cast<gpio_pad*>(PADS_BANK0_BASE + 0x4 + gpio * 4);
}

io_ctrl& get_io_ctrl(unsigned gpio) {
    return *reinterpret_cast<io_ctrl*>(IO_BANK0_BASE + 0x4 + gpio * 8);
}

}  // namespace

export namespace soc::rp2040 {

class gpio : public device::gpio {
 public:
    gpio(std::string const& name) : ::device::gpio(name) {}

    inline void config(unsigned gpio, config_t const& config) override;
    inline void set(unsigned gpio, bool val) override;
    inline bool get(unsigned gpio) override;
};

void gpio::config(unsigned gpio, config_t const& config) {
    if (gpio > 29)
        throw exception("invalid gpio number");

    auto& pad = get_pad(gpio);
    pad.ie = 1;
    pad.od = 0;

    auto& ctrl = get_io_ctrl(gpio);
    ctrl.funcsel = GPIO_FUNC_SIO;

    if (config.dir == dir::OUTPUT)
        reg(GPIO_OE_SET) = 1 << gpio;
    else
        reg(GPIO_OE_CLR) = 1 << gpio;
}

void gpio::set(unsigned gpio, bool val) {
    reg(val ? GPIO_OUT_SET : GPIO_OUT_CLR) = 1 << gpio;
}

bool gpio::get(unsigned gpio) {
    return reg(GPIO_IN) & (1 << gpio);
}

}  // namespace soc::rp2040
