/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2022 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module arch.aarch64.mte;

#include <stdint.h>

export namespace aarch64 {

inline uintptr_t irg(uintptr_t addr) {
    asm volatile("irg %0, %0" : "+r"(addr));
    return addr;
}

inline void stg(uintptr_t addr) {
    asm volatile("stg %0, [%0]" ::"r"(addr));
}

inline uintptr_t ldg(uintptr_t addr) {
    asm volatile("ldg %0, [%0]" : "+r"(addr));
    return addr;
}

}  // namespace aarch64
