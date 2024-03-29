/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>

export module device.uart.pl011;

export import device.uart;
import std.string;
import lib.reg;
import device;
import device.intc;
import core.event;
import lib.time;

using core::event;
using lib::reg::reg32;
using namespace lib::time;

static event rx_evt;

export namespace device {

class pl011 : public uart {
 public:
    struct platform_data {
        uintptr_t base;
        unsigned freq;
        unsigned baudrate;
        unsigned irq;
    };

    pl011(std::string const& name, platform_data const& pdata)
        : uart(name), base(pdata.base), freq(pdata.freq), baud(pdata.baudrate), irq(pdata.irq) {}

    inline void init() override;
    inline void baudrate(unsigned baud) override;
    inline void putc(int c, bool wait = true) override;
    inline int getc(bool wait = true) override;
    inline void flush() override;

 private:
    volatile uint32_t& reg(uint32_t offset) { return reg32(base + offset); }
    void isr();
    uintptr_t base;
    unsigned freq;
    unsigned baud;
    unsigned irq;
};

}  // namespace device

// clang-format off
enum pl011_reg_offset : uint32_t {
    UART_DR     = 0x00,
    UART_FR     = 0x18,
    UART_IBRD   = 0x24,
    UART_FBRD   = 0x28,
    UART_LCR_H  = 0x2c,
    UART_CR     = 0x30,
    UART_IMSC   = 0x38,
    UART_RIS    = 0x3c,
    UART_MIS    = 0x40,
    UART_ICR    = 0x44,
};

enum UART_FR_bits : uint32_t {
    UART_FR_RXEE    = 1 << 4,
    UART_FR_TXFF    = 1 << 5,
    UART_FR_TXFE    = 1 << 7,
};

enum UART_LCR_H_bits : uint32_t {
    UART_LCR_H_FEN	= 1 << 4,
    UART_LCR_H_WLEN8	= 3 << 5,
};

enum UART_CR_bits : uint32_t {
    UART_CR_UARTEN  = 1 << 0,
    UART_CR_TXE     = 1 << 8,
    UART_CR_RXE     = 1 << 9,
};

enum UART_INT_bits : uint32_t {
    UART_RXI        = 1U << 4,
    UART_RTI        = 1U << 6,
};

// clang-format on

namespace device {

void pl011::init() {
    // set baudrate
    baudrate(baud);

    // enable fifo
    reg(UART_LCR_H) = UART_LCR_H_FEN | UART_LCR_H_WLEN8;

    // enable uart
    reg(UART_CR) = UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE;

    auto intc = manager::find<::device::intc>();
    intc->request_irq(
        irq, intc::FLAG_START_ENABLED,
        [](unsigned, void* data) { reinterpret_cast<pl011*>(data)->isr(); }, this);
}

void pl011::baudrate(unsigned baudrate) {
    // baud rate divisor = BAUDDIV = (FUARTCLK/(16x BaudRate))
    uint32_t ibrd = freq / (16 * baudrate);
    uint32_t fbrd = 64 * (freq % (16 * baudrate)) / (16 * baudrate);

    reg(UART_IBRD) = ibrd;
    reg(UART_FBRD) = fbrd;

    // update current baud rate
    baud = baudrate;
}

void pl011::putc(int c, bool wait) {
    if ((reg(UART_FR) & UART_FR_TXFF) && !wait)
        return;

    // wait for room in tx fifo
    // TODO: add spin lock
    while (reg(UART_FR) & UART_FR_TXFF) {}

    reg(UART_DR) = c;
}

int pl011::getc(bool wait) {
    if (!(reg(UART_FR) & UART_FR_RXEE)) {
        return reg(UART_DR);
    }

    if (!wait)
        return 0;

    do {
        // enbable RX interrupt
        reg(UART_IMSC) |= UART_RXI | UART_RTI;

        rx_evt.wait_for_signal();
    } while (reg(UART_FR) & UART_FR_RXEE);

    return reg(UART_DR);
}

void pl011::flush() {
    while (!(reg(UART_FR) & UART_FR_TXFE)) {}
}

void pl011::isr() {
    reg(UART_IMSC) &= ~(UART_RXI | UART_RTI);
    rx_evt.signal();
}

}  // namespace device
