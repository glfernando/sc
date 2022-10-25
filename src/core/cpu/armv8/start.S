/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

#define SCR_EL3_NS      (1U << 0)
#define SCR_EL3_RW      (1U << 10)
#define SCR_EL3_ATA     (1U << 26)
#define HCR_EL2_RW      (1U << 31)
#define HCR_EL2_ATA     (1UL << 56)

.text

.global _start
_start:
        /*
         * Swtich from ELX to EL1
         */

        // read current EL and move to EL1 NS
        mrs     x0, CurrentEL
        tbz     x0, 2, 2f       // test for EL2
        tbz     x0, 3, 1f       // test for EL1

        // configure EL3 registers
        msr     cptr_el3, xzr
        mrs     x0, scr_el3
        orr     x0, x0, SCR_EL3_RW
        orr     x0, x0, SCR_EL3_NS
#ifdef CONFIG_AARCH64_MTE
        orr     x0, x0, SCR_EL3_ATA
#endif
        msr     scr_el3, x0

#ifdef CONFIG_AARCH64_MTE
        // ATA
        mrs	x0, sctlr_el3
        orr	x0, x0, #(1ULL << 43)
        msr	sctlr_el3, x0
#endif

        // configure EL3 -> EL1 switch
        mov     x0, #((0b1111 << 6) | 0b0101)   // EL1h + DAIF  masked
        msr     spsr_el3, x0
        adr     x0, 1f
        msr     elr_el3, x0

2:
#ifdef CONFIG_AARCH64_MTE
        // ATA
        mrs	x0, sctlr_el1
        orr	x0, x0, #(1ULL << 43)
        orr	x0, x0, #(1ULL << 40)
        msr	sctlr_el1, x0
#endif

        // configure EL2 registers
        mrs     x0, hcr_el2
        orr     x0, x0, HCR_EL2_RW
#ifdef CONFIG_AARCH64_MTE
        orr     x0, x0, HCR_EL2_ATA
#endif
        msr     hcr_el2, x0
        msr     cptr_el2, xzr

        // configure EL2 -> EL1 switch
        mov     x0, #((0b1111 << 6) | 0b0101)   // EL1h + DAIF masked
        msr     spsr_el2, x0
        adr     x0, 1f
        msr     elr_el2, x0

        // switch to EL1
        isb
        eret
1:
        // we are running at EL1
        mov     x0, (3 << 20)
        msr     cpacr_el1, x0
        isb

#ifdef CONFIG_AARCH64_MTE
	// set rnd tag seed
        msr	gcr_el1, xzr
        mov	x0, #0x300
        msr	rgsr_el1, x0
#endif

        /*
         * Setup stack
         */
        msr     spsel, #1       // make sure we use sp_el1
        adrp    x5, __stack_end
        add     x5, x5, #:lo12:__stack_end
        mov     sp, x5

        // jump to init
        adrp    x5, init
        add     x5, x5, #:lo12:init
        blr     x5

        // we should never return
        b       .
