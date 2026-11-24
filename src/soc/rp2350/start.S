/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Fernando Lugo <lugo.fernando@gmail.com>
 */

.syntax unified
.cpu cortex-m33
.thumb

.section .boot_vectors, "a"
.align 2
.global __boot_vectors
__boot_vectors:
    .word __stack_end
    .word _soc_entry
    .rept 14
    .word _boot_default_handler
    .endr

.section .picobin_block, "a"
.align 2
.global __picobin_block
__picobin_block:
    .word 0xffffded3    /* PICOBIN_BLOCK_MARKER_START */
    .word 0x10210142    /* IMAGE_TYPE: EXE | ARM | SECURE | RP2350 (1 word) */
    .word 0x000001ff    /* LAST: total preceding item size = 1 word */
    .word 0x00000000    /* relative offset to next block (self-loop) */
    .word 0xab123579    /* PICOBIN_BLOCK_MARKER_END */

.section .reset, "ax"
.align 2
.global _soc_entry
.thumb_func
_soc_entry:
    ldr     r0, =__stack_end
    msr     msp, r0

    /* Enable FPU (CP10 & CP11 full access in SCB->CPACR) */
    ldr     r0, =0xe000ed88
    ldr     r1, [r0]
    orr     r1, r1, #(0xf << 20)
    str     r1, [r0]
    dsb
    isb

    bl      _start

.thumb_func
_boot_default_handler:
    b       _boot_default_handler
