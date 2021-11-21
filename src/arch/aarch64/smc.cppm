/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module arch.aarch64.smc;

export namespace aarch64 {

#pragma clang diagnostic ignored "-Wunused-parameter"
__attribute__((naked)) unsigned long smc(unsigned long x0, unsigned long x1 = 0,
                                         unsigned long x2 = 0, unsigned long x4 = 0) {
    asm volatile(
        "smc #0\n"
        "ret");
}

}  // namespace aarch64
