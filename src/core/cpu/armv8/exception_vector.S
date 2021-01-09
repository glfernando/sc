/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

#include "exception_vector.h"

.section .text

.macro save_regs
        sub     sp, sp, #784
        stp     x0, x1, [sp]
        stp     x2, x3, [sp, #16]
        stp     x4, x5, [sp, #32]
        stp     x6, x7, [sp, #48]
        stp     x8, x9, [sp, #64]
        stp     x10, x11, [sp, #80]
        stp     x12, x13, [sp, #96]
        stp     x14, x15, [sp, #112]
        stp     x16, x17, [sp, #128]
        stp     x18, x19, [sp, #144]
        stp     x20, x21, [sp, #160]
        stp     x22, x23, [sp, #176]
        stp     x24, x25, [sp, #192]
        stp     x26, x27, [sp, #208]
        stp     x28, x29, [sp, #224]
        add     x0, sp, #784
        stp     x30, x0, [sp, #240]
        add     x0, sp, #256
        bl      save_vregs
.endm

.macro restore_regs
        add     x0, sp, #256
        bl      restore_vregs
        ldp     x2, x3, [sp, #16]
        ldp     x4, x5, [sp, #32]
        ldp     x6, x7, [sp, #48]
        ldp     x8, x9, [sp, #64]
        ldp     x10, x11, [sp, #80]
        ldp     x12, x13, [sp, #96]
        ldp     x14, x15, [sp, #112]
        ldp     x16, x17, [sp, #128]
        ldp     x18, x19, [sp, #144]
        ldp     x20, x21, [sp, #160]
        ldp     x22, x23, [sp, #176]
        ldp     x24, x25, [sp, #192]
        ldp     x26, x27, [sp, #208]
        ldp     x28, x29, [sp, #224]
        ldp     x30, x0, [sp, #240]
        mov     x1, sp
        mov     sp, x0
        ldp     x0, x1, [x1]
.endm

.balign 0x800
.global exception_base
exception_base:
        save_regs
        mov     x0, EXCEPTION_TYPE_SP0_SYNC
        mov     x1, sp
        bl      exception_handler
        eret
.balign 0x80
        save_regs
        mov     x0, EXCEPTION_TYPE_SP0_IRQ
        mov     x1, sp
        bl      exception_handler
        eret

.balign 0x80
        save_regs
        mov     x0, EXCEPTION_TYPE_SP0_FIQ
        mov     x1, sp
        bl      exception_handler
        eret
.balign 0x80
        save_regs
        mov     x0, EXCEPTION_TYPE_SP0_SERR
        mov     x1, sp
        bl      exception_handler
        eret
.balign 0x80
        save_regs
        mov     x0, EXCEPTION_TYPE_SPX_SYNC
        mov     x1, sp
        bl      exception_handler
        b       restore_and_return
.balign 0x80
        save_regs
        mov     x0, EXCEPTION_TYPE_SPX_IRQ
        mov     x1, sp
        bl      exception_handler
        b       restore_and_return
.balign 0x80
        save_regs
        mov     x0, EXCEPTION_TYPE_SPX_FIQ
        mov     x1, sp
        bl      exception_handler
        b       restore_and_return
.balign 0x80
        save_regs
        mov     x0, EXCEPTION_TYPE_SPX_SERR
        mov     x1, sp
        bl      exception_handler
        b       restore_and_return
.balign 0x80
        save_regs
        mov     x0, EXCEPTION_TYPE_A64_SYNC
        mov     x1, sp
        bl      exception_handler
        eret
.balign 0x80
        save_regs
        mov     x0, EXCEPTION_TYPE_A64_IRQ
        mov     x1, sp
        bl      exception_handler
	eret
.balign 0x80
        save_regs
        mov     x0, EXCEPTION_TYPE_A64_FIQ
        mov     x1, sp
        bl      exception_handler
        eret
.balign 0x80
        save_regs
        mov     x0, EXCEPTION_TYPE_A64_SERR
        mov     x1, sp
        bl      exception_handler
        eret
.balign 0x80
        save_regs
        mov     x0, EXCEPTION_TYPE_A32_SYNC
        mov     x1, sp
        bl      exception_handler
        eret
.balign 0x80
        save_regs
        mov     x0, EXCEPTION_TYPE_A32_IRQ
        mov     x1, sp
        bl      exception_handler
        eret
.balign 0x80
        save_regs
        mov     x0, EXCEPTION_TYPE_A32_FIQ
        mov     x1, sp
        bl      exception_handler
        eret
.balign 0x80
        save_regs
        mov     x0, EXCEPTION_TYPE_A32_SERR
        mov     x1, sp
        bl      exception_handler
        eret

.balign 0x80
restore_and_return:
        restore_regs
        eret

save_vregs:
        stp     q0, q1, [x0]
        stp     q2, q3, [x0, #0x20]
        stp     q4, q5, [x0, #0x40]
        stp     q6, q7, [x0, #0x60]
        stp     q8, q9, [x0, #0x80]
        stp     q10, q11, [x0, #0xa0]
        stp     q12, q13, [x0, #0xc0]
        stp     q14, q15, [x0, #0xe0]
        stp     q16, q17, [x0, #0x100]
        stp     q18, q19, [x0, #0x120]
        stp     q20, q21, [x0, #0x140]
        stp     q22, q23, [x0, #0x160]
        stp     q24, q25, [x0, #0x180]
        stp     q26, q27, [x0, #0x1a0]
        stp     q28, q29, [x0, #0x1c0]
        stp     q30, q31, [x0, #0x1e0]
        mrs     x1, fpsr
        mrs     x2, fpcr
        add     x0, x0, #0x200
        stp     x1, x2, [x0]
        ret

restore_vregs:
        ldp     q0, q1, [x0]
        ldp     q2, q3, [x0, #0x20]
        ldp     q4, q5, [x0, #0x40]
        ldp     q6, q7, [x0, #0x60]
        ldp     q8, q9, [x0, #0x80]
        ldp     q10, q11, [x0, #0xa0]
        ldp     q12, q13, [x0, #0xc0]
        ldp     q14, q15, [x0, #0xe0]
        ldp     q16, q17, [x0, #0x100]
        ldp     q18, q19, [x0, #0x120]
        ldp     q20, q21, [x0, #0x140]
        ldp     q22, q23, [x0, #0x160]
        ldp     q24, q25, [x0, #0x180]
        ldp     q26, q27, [x0, #0x1a0]
        ldp     q28, q29, [x0, #0x1c0]
        ldp     q30, q31, [x0, #0x1e0]
        add     x0, x0, #0x200
        ldp     x1, x2, [x0]
        msr     fpsr, x1
        msr     fpcr, x2
        ret
