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

    .rodata ALIGN(16) : {
        *(.rodata*);
    }

    .init.array ALIGN(8) : {
        __init_array_start = .;
        KEEP(*(.init_array))
        __init_array_end = .;
    }

    .shell_cmds ALIGN(8): {
        __shell_cmds_start = .;
        KEEP(*(SORT_BY_NAME(.shell_cmds*)))
        __shell_cmds_end = .;
    }

    .eh_frame ALIGN(8): {
        __eh_frame_start = .;
        KEEP(*(.eh_frame))
        __eh_frame_end = .;
    }

    .eh_frame_hdr ALIGN(8): {
        KEEP(*(.eh_frame_hdr))
    }

    __eh_frame_hdr_start = SIZEOF(.eh_frame_hdr) > 0 ? ADDR(.eh_frame_hdr) : 0;
    __eh_frame_hdr_end = SIZEOF(.eh_frame_hdr) > 0 ? . : 0;

    .data ALIGN(4096) : {
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
    __heap_start = .;
    . = . + CONFIG_HEAP_SIZE;
    __heap_end = .;

    . = ALIGN(16);
    __stack_start = .;
    . = . + CONFIG_STACK_SIZE;
    __stack_end = .;

    /DISCARD/ : {
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
