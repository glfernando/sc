/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>

export module lib.servo;

export import lib.exception;
import device.pwm;
import std.type_traits;
import lib.time;

using namespace lib::time;
using lib::exception;

export namespace lib {

template <device::Pwm P>
class servo {
 public:
    constexpr servo(P& pwm, time_ns_t min = 1'000'000, lib::time::time_ns_t max = 2'000'000,
                    unsigned freq = 50)
        : pwm(pwm), min(min), max(max), freq(freq) {}

    void start(unsigned pos = 90) {
        lib::time::time_ns_t period = 1'000'000'000 / freq;
        pwm.period(period);
        position(pos);
        pwm.init();
    }

    void position(unsigned angle) {
        if (angle > 180)
            throw exception("angle cannot be bigger than 180");
        uint64_t delta = max.get_val() - min.get_val();
        uint64_t ns_val = angle * delta / 180;
        pwm.duty_cycle(min + ns_val);
        pos = angle;
    }

    unsigned position() { return pos; }

    void stop() { pwm.deinit(); }

 private:
    P& pwm;
    lib::time::time_ns_t min;
    lib::time::time_ns_t max;
    unsigned freq;
    unsigned pos;
};

}  // namespace lib
