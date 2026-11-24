/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stddef.h>
#include <stdint.h>

export module core.thread.arch;
export import core.cpu.armv8m.exception;

import soc.thread;
import lib.cpu;

__attribute__((naked)) static void thread_init_entry(void) {
    asm volatile(R"(
        mov lr, r4
        mov r0, r5
        bx lr
    )");
}

export namespace core::thread {

constexpr size_t IDLE_THREAD_STACK_SIZE = 1024;

uintptr_t curr_thread[2];

struct context_regs {
#ifdef __ARM_PCS_VFP
    uint32_t s[16];
#endif
    uint32_t r3_pad;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t lr;
};

class thread_arch {
 public:
    void init_context(long pc, long arg, long sp) {
        sp &= ~7L;
        curr_sp = sp - sizeof(context_regs);
        context_regs* regs = reinterpret_cast<context_regs*>(curr_sp);
        regs->lr = reinterpret_cast<uint32_t>(thread_init_entry);
        regs->r4 = static_cast<uint32_t>(pc);
        regs->r5 = static_cast<uint32_t>(arg);
    }

 private:
    uintptr_t curr_sp;
};

void thread_current_addr(uintptr_t addr) {
    curr_thread[lib::cpu::id()] = addr;
}

uintptr_t thread_current_addr() {
    return curr_thread[lib::cpu::id()];
}

__attribute__((naked)) void switch_context(thread_arch* /* tto */, thread_arch* /* tfrom */) {
    // stack pointer is the first element in thread_arch
    asm volatile(
        "push    {r3, r4-r11, lr}\n"
#ifdef __ARM_PCS_VFP
        "vpush   {s16-s31}\n"
#endif
        "mrs     r2, msp\n"
        "str     r2, [r1]\n"
        "ldr     r0, [r0]\n"
        "msr     msp, r0\n"
#ifdef __ARM_PCS_VFP
        "vpop    {s16-s31}\n"
#endif
        "pop     {r3, r4-r11, pc}\n");
}

void arch_init() {}

void arch_idle() {
    asm volatile("wfi");
}

void arch_kick() {
    soc::thread::kick();
}

}  // namespace core::thread
