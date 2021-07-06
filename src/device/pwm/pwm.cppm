/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module device.pwm;

export import device;
import std.string;
import lib.time;

using namespace lib::time;

export namespace device {

class pwm : public device {
 public:
    constexpr static class_type dev_type = class_type::PWM;
    class_type type() const override final { return pwm::dev_type; }

    pwm(std::string const& name) : device(name) {}

    // pwm interface
    virtual void period(time_ns_t period) = 0;
    virtual time_ns_t period() = 0;
    virtual void duty_cycle(time_ns_t cycle) = 0;
    virtual time_ns_t duty_cycle() = 0;
};

// pwm device concept
// This can be use to create generic code that does not use vtable

template <typename T>
concept Pwm = requires(T t, time_ns_t duration) {
    t.period(duration);
    duration = t.period();
    t.duty_cycle(duration);
    duration = t.duty_cycle();
};

};  // namespace device
