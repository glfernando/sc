/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <arch/arm/sysreg.h>
#include <stdint.h>

export module device.intc.nvic;

export import device.intc;

import core.cpu.armv6m.exception;

import std.string;
import std.vector;
import lib.reg;
import lib.fmt;
import lib.exception;

using lib::exception;
using lib::fmt::println;
using lib::fmt::sprint;
using lib::reg::reg32;
using std::string;
using std::vector;

constexpr unsigned EXT_INT_MAX = 32;

export namespace device {

class nvic : public intc {
 public:
    struct platform_data {
        // PPB_BASE
        uintptr_t base;
        unsigned irq_num;
    };

    nvic(string const& name, platform_data const& pdata) : intc(name), base(pdata.base) {
        handlers.resize(pdata.irq_num);
    }

    inline void init() override;
    inline void request_irq(unsigned irq, unsigned flags, handler handler, void* data) override;
    inline void free_irq(unsigned irq) override;
    inline void enable_irq(unsigned irq) override;
    inline void disable_irq(unsigned irq) override;
    inline void send_ipi(unsigned cpu_mask, unsigned irq) override;
    inline void send_ipi(ipi_target target, unsigned irq) override;

 private:
    volatile uint32_t& reg(uint32_t offset) { return reg32(base + offset); }
    struct handler_info {
        handler func;
        void* data;
    };

    static void default_handler() {
        unsigned irq = sysreg_read(ipsr);
        handlers[irq].func(irq, handlers[irq].data);
    }

    uintptr_t base;
    static inline vector<handler_info> handlers;
};

}  // namespace device

namespace device {

// clang-format off
enum nvic_reg_offset : uint32_t {
    NVIC_ISER   = 0xe100,
    NVIC_ICER   = 0xe180,
    NVIC_ISPR   = 0xe200,
    NVIC_ICPR   = 0xe280,
    ICSR        = 0xed04
};

// clang-format on

void nvic::init() {
    // disable and clear all pending interrupts
    reg(NVIC_ICER) = ~0;
    reg(NVIC_ICPR) = ~0;

    // TODO: set priority

    // enable interrupts
    asm volatile("cpsie i");
}

void nvic::request_irq(unsigned irq, unsigned flags, handler handler, void* data) {
    if (irq > handlers.size()) {
        println("invalid irq {}", irq);
        return;
    }
    if (!data) {
        core::cpu::armv6m::exception::register_handler(
            irq, reinterpret_cast<core::cpu::armv6m::exception::handler_t>(handler));
    } else {
        handlers[irq].func = handler;
        handlers[irq].data = data;
        core::cpu::armv6m::exception::register_handler(irq, default_handler);
    }

    if (flags & FLAG_START_ENABLED)
        enable_irq(irq);
}

void nvic::free_irq(unsigned irq) {
    disable_irq(irq);
    handlers[irq].func = nullptr;
    handlers[irq].data = nullptr;
}

void nvic::enable_irq(unsigned irq) {
    if (irq >= 16)
        reg(NVIC_ISER) = 1 << (irq - 16);
}

void nvic::disable_irq(unsigned irq) {
    if (irq >= 16)
        reg(NVIC_ICER) = 1 << (irq - 16);
}

void nvic::send_ipi(unsigned cpu_mask, unsigned irq) {
    if (cpu_mask != 0x1)
        throw exception("ipi only supported for core0");

    switch (irq) {
    case 2:
        reg(ICSR) = 1 << 31;
        break;
    case 14:
        reg(ICSR) = 1 << 28;
        break;
    case 15:
        reg(ICSR) = 1 << 26;
        break;
    case 16 ...(16 + EXT_INT_MAX):
        reg(NVIC_ISPR) = 1 << (irq - 16);
        break;
    default:
        exception(lib::fmt::sprint("invalid irq numner {}", irq));
        break;
    }
}

void nvic::send_ipi(ipi_target target, unsigned irq) {
    if (target == ipi_target::SELF)
        send_ipi(0x1, irq);
}

}  // namespace device
