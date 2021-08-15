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
import soc.rp2040.address_map;
import lib.reg;
import lib.exception;
import device.intc;
import std.vector;

using lib::exception;
using lib::reg::reg32;
using namespace soc::rp2040::address_map;
using std::vector;

namespace {

constexpr unsigned MAX_GPIO_NUM = 29;

// clang-format off
enum reg_offset : uint32_t {
    GPIO_IN         = 0x04,
    GPIO_OUT        = 0x10,
    GPIO_OUT_SET    = 0x14,
    GPIO_OUT_CLR    = 0x18,
    GPIO_OE_SET     = 0x24,
    GPIO_OE_CLR     = 0x28,
};

enum io_bank0_offset : uint32_t {
    INTR0           = 0x0f0,
    PROC0_INTE0     = 0x100,
    PROC0_INTS0     = 0x120,
};

enum drive_strength {
    DRIVE_2mA   = 0,
    DRIVE_4mA   = 1,
    DRIVE_8mA   = 2,
    DRIVE_12mA  = 3,
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
    gpio(std::string const& name, unsigned irq) : ::device::gpio(name), irq(irq) {
        callbacks.resize(MAX_GPIO_NUM + 1);
    }

    inline void init() override;
    inline void config(unsigned gpio, config_t const& config) override;
    inline void set(unsigned gpio, bool val) override;
    inline bool get(unsigned gpio) override;
    inline void register_irq(unsigned gpio, callback cb, void* data) override;

    // clang-format off
    enum class func {
        XIP   = 0,
        SPI   = 1,
        UART  = 2,
        I2C   = 3,
        PWM   = 4,
        SIO   = 5,
        PIO0  = 6,
        PIO1  = 7,
        GPCK  = 8,
        USB   = 9,
    };
    // clang-format on

    void set_func(unsigned gpio, func f);

 private:
    struct cb_info {
        callback cb;
        void* data;
    };
    void isr();
    unsigned irq;
    vector<cb_info> callbacks;
};

}  // namespace soc::rp2040

namespace soc::rp2040 {

void gpio::init() {
    auto intc = ::device::manager::find<::device::intc>();
    intc->request_irq(
        irq, 0, [](unsigned, void* data) { reinterpret_cast<gpio*>(data)->isr(); }, this);
}

void gpio::isr() {
    for (int i = 0; i < 4; ++i) {
        uint32_t status = reg32(IO_BANK0_BASE + i * 4 + PROC0_INTS0);
        uint32_t tmp = status;

        for (int j = 0; tmp; tmp = tmp >> 4, ++j) {
            uint32_t mask = tmp & 0xf;
            if (!mask)
                continue;

            unsigned gpio = i * 8 + j;
            if (gpio > MAX_GPIO_NUM)
                return;
            callbacks[gpio].cb(callbacks[gpio].data);
        }
        reg32(IO_BANK0_BASE + 0 + INTR0 + i * 4) = status;
    }
}

void gpio::config(unsigned gpio, config_t const& config) {
    if (gpio > MAX_GPIO_NUM)
        throw exception("invalid gpio number");

    auto& pad = get_pad(gpio);
    pad.ie = 1;
    pad.od = 0;

    auto& ctrl = get_io_ctrl(gpio);
    ctrl.funcsel = static_cast<uint32_t>(func::SIO);

    pad.pde = config.pull == pull::DOWN;
    pad.pue = config.pull == pull::UP;

    if (config.dir == dir::OUTPUT)
        reg(GPIO_OE_SET) = 1 << gpio;
    else
        reg(GPIO_OE_CLR) = 1 << gpio;

    if (config.trigger == trigger::NONE)
        return;

    uint32_t offset = PROC0_INTE0 + gpio / 8 * 4;
    uint32_t shift = (gpio % 8) * 4;
    uint32_t val = reg32(IO_BANK0_BASE + offset) & ~(0xf << shift);
    switch (config.trigger) {
    case trigger::HIGH:
        val |= 0x2 << shift;
        break;
    case trigger::LOW:
        val |= 0x1 << shift;
        break;
    case trigger::RISING:
        val |= 0x8 << shift;
        break;
    case trigger::FALLING:
        val |= 0x4 << shift;
        break;
    case trigger::BOTH:
        val |= 0xc << shift;
        break;
    case trigger::NONE:
        return;
    }
    reg32(IO_BANK0_BASE + offset) = val;
}

void gpio::set(unsigned gpio, bool val) {
    reg(val ? GPIO_OUT_SET : GPIO_OUT_CLR) = 1 << gpio;
}

bool gpio::get(unsigned gpio) {
    return reg(GPIO_IN) & (1 << gpio);
}

void gpio::register_irq(unsigned gpio, callback cb, void* data) {
    if (gpio > MAX_GPIO_NUM)
        throw exception("invalid gpio number");

    callbacks[gpio].cb = cb;
    callbacks[gpio].data = data;

    auto intc = ::device::manager::find<::device::intc>();
    intc->enable_irq(irq);
}

void gpio::set_func(unsigned gpio, func f) {
    if (gpio > MAX_GPIO_NUM)
        throw exception("invalid gpio number");

    auto& pad = get_pad(gpio);
    pad.ie = 1;
    pad.od = 0;

    auto& ctrl = get_io_ctrl(gpio);
    ctrl.funcsel = static_cast<uint32_t>(f);
}

}  // namespace soc::rp2040
