/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * (C) Copyright 2016, Fernando Lugo <lugo.fernando@gmail.com>
 */

ENTRY(_start)
SECTIONS
{
    . = CONFIG_TEXT_BASE;

    .text : {
        KEEP(*(.loader))
        KEEP(*(.text*))
    }

    .rodata ALIGN(8) : {
        *(.rodata*);
    }

    .data ALIGN(8) : {
        *(.data*)
    }

    .rela.dyn ALIGN(8): {
        __rela_dyn_start = .;
        *(.rela_dyn)
    }
    __rela_dyn_end = .;

    .bss ALIGN(4096) : {
        __bss_start = .;
        *(.bss*);
        . = ALIGN(8);
        __bss_end = .;
    }

    . = ALIGN(16);
    __stack_start = .;
    . = . + CONFIG_STACK_SIZE;
    __stack_end = .;

    /DISCARD/ : {
        *(.eh_frame*)
        *(.comment)
        *(.note*)
        *(.dynsym)
        *(.dynstr*)
        *(.gnu*)
        *(.hash)
        *(.dynamic)
        *(.plt*)
    }
}
