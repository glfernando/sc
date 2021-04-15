/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>

export module soc.rp2040.address_map;

export namespace soc::rp2040::address_map {

// clang-format off
enum : uintptr_t {
    XIP_BASE        = 0x1000'0000,
    SYSINFO_BASE    = 0x4000'0000,
    SYSCFG_BASE     = 0x4000'4000,
    CLOCKS_BASE     = 0x4000'8000,
    RESETS_BASE     = 0x4000'c000,
    PSM_BASE        = 0x4001'0000,
    IO_BANK0_BASE   = 0x4001'4000,
    IO_QSPI_BASE    = 0x4001'8000,
    PADS_BANK0_BASE = 0x4001'c000,
    PADS_QSPI_BASE  = 0x4002'0000,
    XOSC_BASE       = 0x4002'4000,
    PLL_SYS_BASE    = 0x4002'8000,
    PLL_USB_BASE    = 0x4002'c000,
    BUSCTRL_BASE    = 0x4003'0000,
    UART0_BASE      = 0x4003'4000,
    UART1_BASE      = 0x4003'8000,
    SPI0_BASE       = 0x4003'c000,
    SPI1_BASE       = 0x4004'0000,
    I2C0_BASE       = 0x4004'4000,
    I2C1_BASE       = 0x4004'8000,
    ADC_BASE        = 0x4004'c000,
    PWM_BASE        = 0x4005'0000,
    TIMER_BASE      = 0x4005'4000,
    WATCHDOG_BASE   = 0x4005'8000,
    RTC_BASE        = 0x4005'c000,
    ROSC_BASE       = 0x4006'0000,
    TBMAN_BASE      = 0x4006'8000,

    PIO0_BASE       = 0x5020'0000,
    PIO1_BASE       = 0x5030'0000,

    SIO_BASE        = 0xd000'0000,
    PPB_BASE        = 0xe000'0000,
};
// clang-format on

}  // namespace soc::rp2040::address_map
