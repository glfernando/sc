/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

#define R_AARCH64_RELATIVE 1027

.section .loader, "ax"

.global _loader
.type _loader,%function
_loader:
        b _loader_start
.org 0x40, 0x00 // leave room for a header

_loader_start:
        /*
         * Initialize .BSS section
         * assume .bss is at least 8 bytes aligned in the linker script
         */
        adrp    x5, __bss_start
        add     x5, x5, #:lo12:__bss_start
        adrp    x6, __bss_end
        add     x6, x6, #:lo12:__bss_end
1:      cmp     x5, x6
        beq     2f
        str     xzr, [x5], 8
        b       1b
2:
#ifdef RELOCABLE
        /*
         * Runtime relocation
         */
        // read offset
        adr     x7, _loader     // actual load address
        ldr     x8, =CONFIG_TEXT_BASE

        subs    x9, x7, x8
        // FIXME: for some reason we need to relocate even when offset is 0
        //beq     _reloc_done     // offset == 0

         //Lets check if we have data in .rela_dyn, if so then we fix up address in .got
        adrp    x5, __rela_dyn_start
        add     x5, x5, #:lo12:__rela_dyn_start
        adrp    x6, __rela_dyn_end
        add     x6, x6, #:lo12:__rela_dyn_end
        cmp     x5, x6
        beq     _reloc_done     // nothing to relocate

        /*
         * Each entry in .rela is a struct elf64_rela like below
         * struct {
         *      u64 offset
         *      u64 info
         *      u64 addend
         * }
         */
_reloc_loop:
        ldp     x10, x11, [x5], #16     // x10=offset, x11=info
        ldr     x12, [x5], #8           // x12=addend
        cmp     x11, #R_AARCH64_RELATIVE
        bne     _reloc_next             // we only support R_AARCH64_RELATIVE, skip others

        // do the actual relocation Delta(S) + A
        add     x10, x10, x9
        add     x12, x12, x9
        str     x12, [x10]
_reloc_next:
        cmp     x5, x6
        bne     _reloc_loop
_reloc_done:
#endif

        // jump to the entry point
        adrp    x5, _start
        add     x5, x5, #:lo12:_start
        br      x5
