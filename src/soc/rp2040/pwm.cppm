/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>

export module soc.rp2040.pwm;

export import device.pwm;

import std.string;
import soc.rp2040.address_map;
import lib.exception;
import lib.time;

using lib::exception;
using namespace soc::rp2040::address_map;
using namespace lib::time;

namespace {

// TODO: make it configurable
constexpr unsigned CLK_FREQ = 125'000'000;

struct pwm_regs_ {
    union {
        struct {
            uint32_t en : 1;
            uint32_t ph_correct : 1;
            uint32_t a_inv : 1;
            uint32_t b_inv : 1;
            uint32_t divmode : 2;
            uint32_t ph_ret : 1;
            uint32_t ph_adv : 1;
            uint32_t : 24;
        };
        uint32_t data;
    } csr;

    union {
        struct {
            uint32_t frac : 4;
            uint32_t integer : 8;
            uint32_t reserved : 20;
        };
        uint32_t data;
    } div;
    uint32_t ctr;

    union {
        struct {
            uint32_t a : 16;
            uint32_t b : 16;
        };
        uint32_t data;
    } cc;
    uint32_t top;
};

using pwm_regs = volatile pwm_regs_;

static_assert(sizeof(pwm_regs) == 5 * sizeof(int));

// clang-format off
enum pwm_function {
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

struct pwm_pad {
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

pwm_regs& get_pwm_regs(unsigned gpio) {
    if (gpio > 29)
        throw exception("invalid pwm number");
    else if (gpio > 15)
        gpio -= 16;

    return *reinterpret_cast<pwm_regs*>(PWM_BASE + (gpio >> 1) * 0x14);
}

pwm_pad& get_pad(unsigned pwm) {
    return *reinterpret_cast<pwm_pad*>(PADS_BANK0_BASE + 0x4 + pwm * 4);
}

io_ctrl& get_io_ctrl(unsigned pwm) {
    return *reinterpret_cast<io_ctrl*>(IO_BANK0_BASE + 0x4 + pwm * 8);
}

}  // namespace

export namespace soc::rp2040 {

class pwm : public device::pwm {
 public:
    pwm(std::string const& name, unsigned gpio) : ::device::pwm(name), gpio(gpio), p(0), dc(0) {}

    inline void init() override;
    inline void period(time_ns_t period) override;
    inline time_ns_t period() override;
    inline void duty_cycle(time_ns_t cycle) override;
    inline time_ns_t duty_cycle() override;

 private:
    unsigned gpio;
    // unsigned clk_freq;
    time_ns_t p;
    time_ns_t dc;
    unsigned ns_per_cnt;
};

void pwm::init() {
    auto& pwm = get_pwm_regs(gpio);

    auto& pad = get_pad(gpio);
    pad.ie = 1;
    pad.od = 0;

    auto& ctrl = get_io_ctrl(gpio);
    ctrl.funcsel = GPIO_FUNC_PWM;

    if (p == 0) {
        // initialize divisor as 1
        pwm.div.integer = 1;
        pwm.div.frac = 0;
        pwm.top = 0xffff;

        ns_per_cnt = 1'000'000'000 / CLK_FREQ;
        p = ns_per_cnt * pwm.top;
    }

    pwm.csr.en = 1;
}

void pwm::period(time_ns_t period) {
    auto& pwm = get_pwm_regs(gpio);
    p = period;
    if (p == 20ms) {
        // standard servo 20ms val, use div = 250 and top == 10'000
        pwm.div.frac = 0;
        pwm.div.integer = 250;
        pwm.top = 10'000;
        ns_per_cnt = 2'000;
        p = ns_per_cnt * pwm.top;
    }
}

time_ns_t pwm::period() {
    return p;
}

void pwm::duty_cycle(time_ns_t cycle) {
    if (cycle > p)
        throw exception("duty cycle cannot be bigger than the period");

    dc = cycle;
    auto& pwm = get_pwm_regs(gpio);
    // check which challen the gpio is connected to
    if (gpio % 2)
        pwm.cc.b = dc.get_val() / ns_per_cnt;
    else
        pwm.cc.a = dc.get_val() / ns_per_cnt;
}

time_ns_t pwm::duty_cycle() {
    return dc;
}

}  // namespace soc::rp2040
