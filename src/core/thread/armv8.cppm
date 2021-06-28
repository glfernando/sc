/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <arch/aarch64/sysreg.h>
#include <stdint.h>
#include <string.h>

export module core.thread.arch;
export import core.cpu.armv8.exception;

__attribute__((naked)) static void thread_init_isr(void) {
    asm volatile(R"(
        mov lr, x18
        mov x0, x19
        mov x1, xzr
        mov x2, xzr
        mov x3, xzr
        mov x4, xzr
        mov x5, xzr
        mov x6, xzr
        mov x7, xzr
        mov x8, xzr
        mov x9, xzr
        mov x10, xzr
        mov x11, xzr
        mov x12, xzr
        mov x13, xzr
        mov x14, xzr
        mov x15, xzr
        mov x16, xzr
        mov x17, xzr
        mov x18, xzr
        mov x19, xzr
        mov x20, xzr
        mov x21, xzr
        mov x22, xzr
        mov x23, xzr
        mov x24, xzr
        mov x25, xzr
        mov x26, xzr
        mov x27, xzr
        mov x28, xzr
        mov x29, xzr
        ret
    )");
}

struct context_regs {
    unsigned long x18;
    unsigned long x19;
    unsigned long x20;
    unsigned long x21;
    unsigned long x22;
    unsigned long x23;
    unsigned long x24;
    unsigned long x25;
    unsigned long x26;
    unsigned long x27;
    unsigned long x28;
    unsigned long x29;
    unsigned long x30;
    unsigned long unused;
    /* vector register */
    unsigned long q8;
    unsigned long q9;
    unsigned long q10;
    unsigned long q11;
    unsigned long q12;
    unsigned long q13;
    unsigned long q14;
    unsigned long q15;
};

export namespace core::thread {

constexpr size_t IDLE_THREAD_STACK_SIZE = 4096;

class thread_arch {
 public:
    void init_context(long pc, long arg, long sp) {
        curr_sp = sp - sizeof(context_regs);
        context_regs* regs = reinterpret_cast<context_regs*>(curr_sp);
        memset(regs, 0, sizeof *regs);
        regs->x30 = reinterpret_cast<unsigned long>(thread_init_isr);
        regs->x18 = pc;
        regs->x19 = arg;
        regs->x20 = sysreg_read(spsr_el1);
    }

 private:
    uintptr_t curr_sp;
};

void thread_current_addr(uintptr_t addr) {
    sysreg_write(tpidr_el1, addr);
}

uintptr_t thread_current_addr() {
    return sysreg_read(tpidr_el1);
}

__attribute__((naked)) void switch_context(thread_arch* /* tto */, thread_arch* /* tfrom */) {
    asm volatile(R"(
        sub     sp, sp, %0
        stp     x18, x19, [sp]
        stp     x20, x21, [sp,#0x10]
        stp     x22, x23, [sp,#0x20]
        stp     x24, x25, [sp,#0x30]
        stp     x26, x27, [sp,#0x40]
        stp     x28, x29, [sp,#0x50]
        stp     x30, xzr, [sp,#0x60]
        stp      d8,  d9, [sp, #0x70]
        stp     d10, d11, [sp, #0x80]
        stp     d12, d13, [sp, #0x90]
        stp     d14, d15, [sp, #0xa0]
        mov     x2, sp
        str     x2, [x1]
        ldr     x2, [x0]
        mov     sp, x2
        ldp     x18, x19, [sp], #16
        ldp     x20, x21, [sp], #16
        ldp     x22, x23, [sp], #16
        ldp     x24, x25, [sp], #16
        ldp     x26, x27, [sp], #16
        ldp     x28, x29, [sp], #16
        ldp     x30,  x0, [sp], #16
        ldp      d8,  d9, [sp], #16
        ldp     d10, d11, [sp], #16
        ldp     d12, d13, [sp], #16
        ldp     d14, d15, [sp], #16
        ret
    )" ::"I"(sizeof(context_regs)));
}

void arch_idle() {
    asm volatile("wfi");
}

}  // namespace core::thread
