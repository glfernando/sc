/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <app/shell.h>
#include <endian.h>
#include <stdint.h>

export module lib.ft6206;

import device.i2c;
import lib.gpio;
import lib.fmt;
import std.concepts;
import std.vector;
import lib.async;
import core.event;

using core::event;
using lib::async;
using lib::fmt::println;
using std::vector;

enum reg_offset : uint8_t {
    TD_STATUS = 0x02,
    P1_XH = 0x03,
    P1_XL = 0x04,
    P1_YH = 0x05,
    P1_YL = 0x06,
};

enum event_flag {
    PRESS_DOWN = 0x0,
    LIFT_UP = 0x1,
    CONTACT = 0x2,
    NONE = 0x3,
};

static device::i2c* bus;
static uint8_t addr;
uint16_t width;
uint16_t height;

uint16_t read16(uint8_t offset) {
    uint16_t val;
    bus->write(addr, &offset, 1, true);
    bus->read(addr, reinterpret_cast<uint8_t*>(&val), sizeof val);
    return be16(val);
}

template <std::integral T>
void write(uint8_t offset, T const val) {
    uint8_t buf[sizeof val + 1];
    buf[0] = offset;
    memcpy(&buf[1], &val, sizeof val);
    bus->write(addr, buf, sizeof buf);
}

template <typename T>
std::decay_t<T> decay_copy(T&& v) {
    return std::forward<T>(v);
}

class ft_cb {
 public:
    virtual void operator()(uint8_t, uint16_t, uint16_t) = 0;
    virtual ~ft_cb() {}
    uint8_t event_mask;
};

template <typename F>
class ft_cb_wrapper : public ft_cb {
 public:
    ft_cb_wrapper(F&& f) : func{decay_copy(std::forward<F>(f))} {}

    void operator()(uint8_t e, uint16_t x, uint16_t y) override { return func(e, x, y); }

 private:
    std::decay_t<F> func;
};

static vector<ft_cb*> callbacks;
void isr();
void notify_thread();
static event e;

export namespace lib::ft6206 {

void init(device::i2c* dev, uint8_t saddr, unsigned irq_gpio, uint16_t w, uint16_t h) {
    bus = dev;
    addr = saddr;
    width = w;
    height = h;

    lib::gpio::config_t irq_config = {
        .dir = lib::gpio::dir::INPUT,
        .trigger = lib::gpio::trigger::FALLING,
        .pull = lib::gpio::pull::UP,
    };

    lib::gpio::config(irq_gpio, irq_config);
    lib::gpio::register_irq(irq_gpio, isr);

    new async{"ft_thread", notify_thread};
}

template <typename F>
void register_callback(F&& f, uint8_t event_mask) {
    ft_cb* cb = new ft_cb_wrapper(std::forward<F>(f));
    cb->event_mask = event_mask;

    callbacks.push_back(cb);
}

};  // namespace lib::ft6206

static event_flag prev_evt = NONE;
static uint16_t x;
static uint16_t y;

void notify_thread() {
    for (;;) {
        e.wait_for_signal();
        for (auto cb : callbacks) {
            if (cb->event_mask & prev_evt)
                (*cb)(prev_evt, x, y);
        }
    }
}

void isr() {
    x = read16(P1_XH);
    event_flag evt = static_cast<event_flag>((x >> 14) & 0x3);
    y = read16(P1_YH) & 0xfff;
    x = x & 0xfff;
    x = width - x;
    y = height - y;
    if (prev_evt != evt) {
        if (!callbacks.empty())
            e.signal();
    }
    // println("x={}, y={}, e={}", x, y, static_cast<int>(evt));
    prev_evt = evt;
}
