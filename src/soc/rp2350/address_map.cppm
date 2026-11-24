/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>

export module soc.rp2350.address_map;

export namespace soc::rp2350::address_map {

// clang-format off
enum : uintptr_t {
    XIP_BASE        = 0x1000'0000,
    SYSINFO_BASE    = 0x4000'0000,
    SYSCFG_BASE     = 0x4000'8000,
    CLOCKS_BASE     = 0x4001'0000,
    PSM_BASE        = 0x4001'8000,
    RESETS_BASE     = 0x4002'0000,
    IO_BANK0_BASE   = 0x4002'8000,
    IO_QSPI_BASE    = 0x4003'0000,
    PADS_BANK0_BASE = 0x4003'8000,
    PADS_QSPI_BASE  = 0x4004'0000,
    XOSC_BASE       = 0x4004'8000,
    PLL_SYS_BASE    = 0x4005'0000,
    PLL_USB_BASE    = 0x4005'8000,
    BUSCTRL_BASE    = 0x4006'8000,
    UART0_BASE      = 0x4007'0000,
    UART1_BASE      = 0x4007'8000,
    SPI0_BASE       = 0x4008'0000,
    SPI1_BASE       = 0x4008'8000,
    I2C0_BASE       = 0x4009'0000,
    I2C1_BASE       = 0x4009'8000,
    ADC_BASE        = 0x400a'0000,
    PWM_BASE        = 0x400a'8000,
    TIMER0_BASE     = 0x400b'0000,
    TIMER1_BASE     = 0x400b'8000,
    WATCHDOG_BASE   = 0x400d'8000,
    ROSC_BASE       = 0x400e'8000,
    TICKS_BASE      = 0x4010'8000,

    PIO0_BASE       = 0x5020'0000,
    PIO1_BASE       = 0x5030'0000,
    PIO2_BASE       = 0x5040'0000,

    SIO_BASE        = 0xd000'0000,
    PPB_BASE        = 0xe000'0000,
};
// clang-format on

}  // namespace soc::rp2350::address_map
