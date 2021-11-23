/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * (C) Copyright 2016, Fernando Lugo <lugo.fernando@gmail.com>
 */

MEMORY
{
    FLASH(rx) : ORIGIN = 0x10000000, LENGTH = 2048k
    RAM(rwx) : ORIGIN =  0x20000000, LENGTH = 256k
    SCRATCH_X(rwx) : ORIGIN = 0x20040000, LENGTH = 4k
    SCRATCH_Y(rwx) : ORIGIN = 0x20041000, LENGTH = 4k
}

REGION_ALIAS("REGION_TESTS", RAM);

ENTRY(_soc_entry)
SECTIONS
{
    .boot2 ORIGIN(FLASH) : {
        __boot2_start= .;
        KEEP(*(.boot2))
        __boot2_end = .;
    } > FLASH

    .text : {
        KEEP(*(.reset))
        *(.text .text.*)
        *(.gnu.linkonce.t.*)
    } > FLASH

    .rodata ALIGN(16) : {
        *(.rodata .rodata.*);
        *(.gnu.linkonce.r.*);
    } > FLASH

    .preinit.array ALIGN(8) : {
        __preinit_array_start = .;
        KEEP(*(.preinit_array))
        __preinit_array_end = .;
    } > FLASH

    .init.array ALIGN(8) : {
        __init_array_start = .;
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        __init_array_end = .;
    } > FLASH

    .got : {
        *(.got*)
    } > FLASH

    .shell_cmds ALIGN(8): {
        __shell_cmds_start = .;
        KEEP(*(SORT_BY_NAME(.shell_cmds*)))
        __shell_cmds_end = .;
    } > FLASH

    .ARM.extab : {
        KEEP(*(.ARM.extab* .gnu.linkonce.armextab.*))
    } > FLASH

    __exidx_start = .;
    .ARM.exidx :
    {
        KEEP(*(.ARM.exidx* .gnu.linkonce.armexidx.*))
    } > FLASH
    __exidx_end = .;

    .data ALIGN(256): {
        __data_start = .;
        KEEP(*(.vector_table))
        *(.data .data.*)
        . = ALIGN(16);
        __data_end = .;
    } > RAM AT> FLASH

    __fdata = LOADADDR(.data);

    .bss : {
        __bss_start = .;
        *(.bss .bss.*);
        . = ALIGN(16);
        __bss_end = .;
    } > RAM

    __heap_start = .;
    . = . + LENGTH(RAM) - SIZEOF(.data) - SIZEOF(.bss) - CONFIG_STACK_SIZE;
    __heap_end = .;

    __stack_start = .;
    . = . + CONFIG_STACK_SIZE;
    __stack_end = ORIGIN(RAM) + LENGTH(RAM);

    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.dynsym)
        *(.dynstr*)
        *(.gnu*)
        *(.hash)
        *(.dynamic)
        *(.plt*)
        *(.rela_dyn*)
    }
}
