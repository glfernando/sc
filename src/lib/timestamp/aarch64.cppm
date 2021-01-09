/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module lib.timestamp.aarch64;

#include <arch/aarch64/sysreg.h>
#include <stdint.h>

export namespace sc::lib::timestamp {

uint64_t ticks() {
    return sysreg_read(cntpct_el0);
}

uint64_t freq() {
    return sysreg_read(cntfrq_el0);
}

};  // namespace sc::lib::timestamp
