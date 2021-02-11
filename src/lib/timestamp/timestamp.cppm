/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>

export module lib.timestamp;

export import lib.timestamp.arch;

export namespace sc::lib::timestamp {

uint64_t ticks_to_xs(uint64_t ticks, uint64_t factor) {
    return ticks * factor / freq();
}

uint64_t ticks_to_ms(uint64_t ticks) {
    return ticks_to_xs(ticks, 1000);
}

uint64_t ticks_to_us(uint64_t ticks) {
    return ticks_to_xs(ticks, 1000'000);
}

uint64_t ticks_to_ns(uint64_t ticks) {
    return ticks_to_xs(ticks, 1000'000'000);
}

uint64_t ms() {
    return ticks_to_ms(ticks());
}

uint64_t us() {
    return ticks_to_us(ticks());
}

uint64_t ns() {
    return ticks_to_ns(ticks());
}

}  // namespace sc::lib::timestamp
