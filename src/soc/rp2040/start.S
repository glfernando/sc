

.cpu cortex-m0plus
.thumb

.section .reset, "ax"
        b _real_start
        nop

.global _soc_entry
_soc_entry:
        movs    r0, #0
        ldr     r1, =(0xe0000000 + 0x0000ed00)
        str     r0, [r1, #8]
        ldmia   r0!, {r1, r2}
        bx      r2

_real_start:
        ldr     r0, =__stack_end
        msr     msp, r0
        bl _start
