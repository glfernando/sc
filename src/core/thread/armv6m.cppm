/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stddef.h>
#include <stdint.h>

export module core.thread.arch;

__attribute__((naked)) static void thread_init_entry(void) {
    asm volatile(R"(
        mov lr, r4
        mov r0, r5
        bx lr
    )");
}

export import core.cpu.armv6m.exception;

export namespace core::thread {

constexpr size_t IDLE_THREAD_STACK_SIZE = 1024;

struct context_regs {
    unsigned r8;
    unsigned r10;
    unsigned r11;
    unsigned lr;
    unsigned r4;
    unsigned r5;
    unsigned r6;
    unsigned r7;
};

class thread_arch {
 public:
    void init_context(long pc, long arg, long sp) {
        curr_sp = sp - sizeof(context_regs);
        context_regs* regs = reinterpret_cast<context_regs*>(curr_sp);
        regs->lr = reinterpret_cast<unsigned long>(thread_init_entry);
        regs->r4 = pc;
        regs->r5 = arg;
    }

 private:
    uintptr_t curr_sp;
};

void thread_current_addr(uintptr_t addr) {
    asm volatile("mov r9, %0" ::"r"(addr));
}

uintptr_t thread_current_addr() {
    uintptr_t reg;
    asm volatile("mov %0, r9" : "=r"(reg));
    return reg;
}

__attribute__((naked)) void switch_context(thread_arch* /* tto */, thread_arch* /* tfrom */) {
    // stack pointer is the first element in thread_arch
    asm volatile(R"(
        push    {r4-r7}
        mov     r4, r8
        mov     r5, r10
        mov     r6, r11
        push    {r4-r6, lr}
        mrs     r2, msp
        str     r2, [r1]
        ldr     r0, [r0]
        ldmia   r0!, {r4-r7}
        mov     r8, r4
        mov     r10, r5
        mov     r11, r6
        mov     lr, r7
        ldmia   r0!, {r4-r7}
        msr     msp, r0
        bx      lr
    )");
}

void arch_idle() {
    asm volatile("wfi");
}

}  // namespace core::thread
