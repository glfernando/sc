/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>
#include <string.h>

extern "C" [[noreturn]] void init();

export module core.cpu.armv6m.start;

export namespace core::cpu {

extern "C" uint8_t __bss_start[];
extern "C" uint8_t __bss_end[];
extern "C" uint8_t __fdata[];
extern "C" uint8_t __data_start[];
extern "C" uint8_t __data_end[];

extern "C" void _start() {
    // init .bss section
    memset(__bss_start, 0, __bss_end - __bss_start);

    // move data from flash to RAM
    memcpy(__data_start, __fdata, __data_end - __data_start);

    init();
}

}  // namespace core::cpu
