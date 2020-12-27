/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>

export module lib.reg;

export namespace sc::lib::reg {

volatile uint8_t& reg8(uintptr_t addr)
{
    return *reinterpret_cast<uint8_t*>(addr);
}

volatile uint16_t& reg16(uintptr_t addr)
{
    return *reinterpret_cast<uint16_t*>(addr);
}

volatile uint32_t& reg32(uintptr_t addr)
{
    return *reinterpret_cast<uint32_t*>(addr);
}

volatile uint64_t& reg64(uintptr_t addr)
{
    return *reinterpret_cast<uint64_t*>(addr);
}

}
