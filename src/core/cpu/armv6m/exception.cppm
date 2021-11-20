/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <arch/arm/sysreg.h>
#include <libunwind.h>
#include <stdint.h>

export module core.cpu.armv6m.exception;

import core.cpu.armv6m.start;
import lib.fmt;
import lib.reg;
import lib.backtrace;
import lib.cpu;
import lib.exception;
import std.string;

using lib::backtrace;
using lib::fmt::println;
using lib::fmt::sprint;
using lib::reg::reg32;
using std::string;

namespace core::cpu::armv6m::exception {

constexpr unsigned EXT_INT_MAX = 32;

export using handler_t = void(*)(void);

struct vector_table_t {
    void* sp_main;
    handler_t reset;
    handler_t nmi;
    handler_t hard_fault;
    handler_t mem_manage;
    handler_t bus_fault;
    handler_t usage_fault;
    handler_t reserved[4];
    handler_t svc;
    handler_t debug_monitor;
    handler_t reserved1;
    handler_t pend_sv;
    handler_t systick;
    volatile handler_t ext_int[EXT_INT_MAX];
};

enum class type {
    NMI,
    HARD_FAULT,
    MEM_MANAGE,
    BUS_FAULT,
    USAGE_FAULT,
};

static char const* type_to_str(type type) {
    switch (type) {
    case type::NMI:
        return "nmi";
    case type::HARD_FAULT:
        return "hard fault";
    case type::MEM_MANAGE:
        return "mem manage";
    case type::BUS_FAULT:
        return "bus fault";
    case type::USAGE_FAULT:
        return "usage fault";
    default:
        return "unknown";
    }
}

export extern "C" uint8_t __stack_end[];

struct excep_frame {
    unsigned r8;
    unsigned r9;
    unsigned r10;
    unsigned r11;
    unsigned r4;
    unsigned r5;
    unsigned r6;
    unsigned r7;
    unsigned r0;
    unsigned r1;
    unsigned r2;
    unsigned r3;
    unsigned r12;
    unsigned lr;
    unsigned pc;
    unsigned xpsr;
};

static void fill_unw_context(unw_context_t& uc, excep_frame& regs) {
    auto data = reinterpret_cast<uint32_t*>(uc.data);
    data[7] = regs.r7;
    data[13] = reinterpret_cast<unsigned>(&regs) + sizeof regs;
    // consider SP aligment
    if (regs.xpsr & (1 << 9))
        data[13] += 4;
    data[14] = regs.lr;
    data[15] = regs.pc;
}

static void unhandled_excep(excep_frame& regs, unsigned exc_return) {
    println("unhandled exception number {} at core {}", sysreg_read(ipsr), lib::cpu::id());

    unsigned sp = reinterpret_cast<unsigned>(&regs) + sizeof regs;
    println("\nregisters:");
    println(" r0: {:#08x}    r1: {:#08x}    r2: {:#08x}    r3: {:#08x}", regs.r0, regs.r1, regs.r2,
            regs.r4);
    println(" r4: {:#08x}    r5: {:#08x}    r6: {:#08x}    r7: {:#08x}", regs.r4, regs.r5, regs.r6,
            regs.r7);
    println(" r8: {:#08x}    r9: {:#08x}   r10: {:#08x}   r11: {:#08x}", regs.r8, regs.r9, regs.r10,
            regs.r11);
    println("r12: {:#08x}    sp: {:#08x}    lr: {:#08x}    pc: {:#08x}", regs.r12, sp, regs.lr,
            regs.pc);

    println("\nxpsr: {:#08x}   exc_return: {:#08x}", regs.xpsr, exc_return);

    println("\nbacktrace:");
    unw_context_t uc;
    fill_unw_context(uc, regs);
    backtrace(&uc);

    for (;;)
        asm volatile("wfe");
}

// clang-format off
__attribute__((naked)) static void default_handler() {
    asm volatile(
        "mov    r0, r8\n"
        "mov    r1, r9\n"
        "mov    r2, r10\n"
        "mov    r3, r11\n"
        "push   {r0-r7}\n");
    asm volatile(
        "mov    r0, sp\n"
        "mov    r1, lr\n"
        "bx     %0\n"
        :: "r"(unhandled_excep) : "r0", "r1");
}
// clang-format on

volatile static vector_table_t vector_table __attribute__((section(".vector_table")))
__attribute__((used)) __attribute__((aligned(256))) = {
    .sp_main = __stack_end,
    .reset = _start,
    .nmi = default_handler,
    .hard_fault = default_handler,
};

}  // namespace core::cpu::armv6m::exception

export namespace core::cpu::armv6m::exception {

void init() {
    // set VTOR address
    reg32(0xe0000000 + 0xed08) = reinterpret_cast<uintptr_t>(&vector_table);
}

void register_handler(unsigned num, handler_t handler) {
    switch (num) {
    case 2:
        vector_table.nmi = handler;
        break;
    case 3:
        vector_table.hard_fault = handler;
        break;
    case 11:
        vector_table.svc = handler;
        break;
    case 14:
        vector_table.pend_sv = handler;
        break;
    case 15:
        vector_table.systick = handler;
        break;
    case 16 ...(16 + EXT_INT_MAX):
        vector_table.ext_int[num - 16] = handler;
        break;
    default:
        throw lib::exception(sprint("invalid vector number {}", num));
    }
}

}  // namespace core::cpu::armv6m::exception
