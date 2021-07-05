/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module device.timer;

export import device;
import std.string;
import lib.time;

export namespace device {

class timer : public device {
 public:
    constexpr static class_type dev_type = class_type::TIMER;
    class_type type() const override final { return timer::dev_type; }

    timer(std::string const& name) : device(name) {}

    // even place holder, code implementing timer class interface should use this as a base for a
    // vendor timer specific event struct
    struct event {};

    enum class type {
        ONE_SHOT,
        PERIODIC,
    };

    using callback = void (*)(void*);

    virtual event* create(enum type, callback cb, void* data) = 0;
    virtual void set(event* e, lib::time::time_us_t period) = 0;
    virtual void cancel(event* e) = 0;
    virtual void destroy(event* e) = 0;
};

template <typename T>
concept Timer = requires(T t, timer::callback cb, void* data, enum timer::type type,
                         timer::event* e, lib::time::time_us_t period) {
    e = t.create(type, cb, data);
    t.set(e, period);
    t.cancel(e);
    t.destroy(e);
};

}  // namespace device
