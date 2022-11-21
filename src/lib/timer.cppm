/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <errcodes.h>

export module lib.timer;

export import lib.time;

export import device.timer;
export import std.type_traits;
import lib.fmt;
import lib.exception;
import lib.lock;

using lib::exception;
using lib::lock;
using lib::slock_irqsafe;
using lib::fmt::println;
using lib::time::time_us_t;

namespace {

static device::timer* timer_dev;
static lock timer_lock;

device::timer& get_timer() {
    slock_irqsafe guard{timer_lock};
    if (timer_dev) {
        return *timer_dev;
    }

    timer_dev = device::manager::find<::device::timer>();
    if (!timer_dev) {
        throw exception("no timer", ERR_NOT_FOUND);
    }
    return *timer_dev;
}

}  // namespace

export namespace lib {

template <typename T>
std::decay_t<T> decay_copy(T&& v) {
    return std::forward<T>(v);
}

class timer_cb {
 public:
    virtual void operator()() = 0;
    virtual ~timer_cb() {}
};

template <typename F>
class timer_cb_wrapper : public timer_cb {
 public:
    timer_cb_wrapper(F&& f) : func{decay_copy(std::forward<F>(f))} {}

    void operator()() override { return func(); }

 private:
    std::decay_t<F> func;
};

class timer {
 public:
    enum class type {
        ONE_SHOT,
        PERIODIC,
    };

    timer(type type = type::PERIODIC) : cb(nullptr), e(nullptr), type(type) {}
    template <typename F>
    void start(F&& f, time_us_t period);
    void start(time_us_t period);
    void stop();
    ~timer();

 private:
    timer_cb* cb;
    device::timer::event* e;
    type type;
};

}  // namespace lib

export namespace lib {

template <typename F>
void timer::start(F&& f, time_us_t period) {
    auto& dev = get_timer();
    cb = new timer_cb_wrapper(std::forward<F>(f));
    auto dev_type =
        type == type::ONE_SHOT ? device::timer::type::ONE_SHOT : device::timer::type::PERIODIC;
    if (!e) {
        // create event
        e = dev.create(
            dev_type,
            [](void* data) {
                auto self = reinterpret_cast<timer*>(data);
                (*(self->cb))();
            },
            this);
    }
    dev.set(e, period);
}

void timer::start(time_us_t period) {
    if (cb == nullptr)
        throw exception("no callback set");
    auto& dev = get_timer();
    dev.set(e, period);
}

void timer::stop() {
    auto& dev = get_timer();
    dev.cancel(e);
}

timer::~timer() {
    if (e) {
        auto& dev = get_timer();
        dev.destroy(e);
        e = nullptr;
    }
}

}  // namespace lib
