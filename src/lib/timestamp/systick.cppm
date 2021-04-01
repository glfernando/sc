/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module lib.timestamp.arch;

#include <stdint.h>

export namespace lib::timestamp {

uint64_t ticks() {
    return 0;
}

uint64_t freq() {
    return 1;
}

};  // namespace lib::timestamp
