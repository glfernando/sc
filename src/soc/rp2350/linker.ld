/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Fernando Lugo <lugo.fernando@gmail.com>
 */

MEMORY
{
    FLASH(rx) : ORIGIN = 0x10000000, LENGTH = 4096k
    RAM(rwx) : ORIGIN = 0x20000000, LENGTH = 512k
    SCRATCH_X(rwx) : ORIGIN = 0x20080000, LENGTH = 4k
    SCRATCH_Y(rwx) : ORIGIN = 0x20081000, LENGTH = 4k
}

REGION_ALIAS("REGION_TESTS", FLASH);

ENTRY(_soc_entry)
SECTIONS
{
    .boot_vectors ORIGIN(FLASH) : {
        KEEP(*(.boot_vectors))
    } > FLASH

    .picobin_block ALIGN(4) : {
        KEEP(*(.picobin_block))
        __picobin_end = .;
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

    .data ALIGN(512) : {
        __data_start = .;
        KEEP(*(.vector_table))
        *(.data .data.*)
        . = ALIGN(16);
        __data_end = .;
    } > RAM AT> FLASH

    __fdata = LOADADDR(.data);

    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss .bss.*);
        *(COMMON);
        . = ALIGN(16);
        __bss_end = .;
    } > RAM

    . = ALIGN(16);
    __heap_start = .;
    __heap_end = ORIGIN(RAM) + LENGTH(RAM) - CONFIG_STACK_SIZE;

    __stack_start = __heap_end;
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

ASSERT(__picobin_end <= ORIGIN(FLASH) + 4096, "IMAGE_DEF block must lie within first 4KB of flash")
ASSERT(__heap_start < __heap_end, "RAM overflow: data/bss exceeds RAM")
