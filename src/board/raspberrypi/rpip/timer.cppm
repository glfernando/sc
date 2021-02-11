/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

// TODO: finish implemenation and move under src/soc/rp2040/

module;

//#include <stdint.h>

export module device.timer.rp2040;
export import device.timer;

import std.string;
import lib.time;

using sc::lib::time::time_us_t;
using std::string;

export namespace device {

class timer_rp2040 : public timer {
 public:
    timer_rp2040(string const& name) : timer(name) {}

    inline void init() override {}
    inline event* create(enum type, callback, void*) override { return nullptr; }
    inline void set(event*, time_us_t) override {}
    inline void cancel(event*) override {}
    inline void destroy(event*) override {}
};

}  // namespace device
