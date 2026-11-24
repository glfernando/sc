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

#ifdef ARMV8M
import core.cpu.armv8m.exception;
#else
import core.cpu.armv6m.exception;
#endif

import std.string;
import std.vector;
import lib.reg;
import lib.fmt;
import lib.exception;

#ifdef ARMV8M
namespace cpu_excep = core::cpu::armv8m::exception;
#else
namespace cpu_excep = core::cpu::armv6m::exception;
#endif

using lib::exception;
using lib::fmt::println;
using lib::fmt::sprint;
using lib::reg::reg32;
using std::string;
using std::vector;

export namespace device {

class nvic : public intc {
 public:
    struct platform_data {
        // PPB_BASE
        uintptr_t base;
        unsigned irq_num;
    };

    nvic(string const& name, platform_data const& pdata)
        : intc(name),
          base(pdata.base),
          ext_irq_num(pdata.irq_num > 16 ? pdata.irq_num - 16 : 0) {
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
    unsigned ext_irq_num;
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
    unsigned words = (ext_irq_num + 31) / 32;
    for (unsigned i = 0; i < words; ++i) {
        reg(NVIC_ICER + i * 4) = ~0U;
        reg(NVIC_ICPR + i * 4) = ~0U;
    }

    // TODO: set priority

    // enable interrupts
    asm volatile("cpsie i");
}

void nvic::request_irq(unsigned irq, unsigned flags, handler handler, void* data) {
    if (irq >= handlers.size()) {
        println("invalid irq {}", irq);
        return;
    }
    if (!data) {
        cpu_excep::register_handler(
            irq, reinterpret_cast<cpu_excep::handler_t>(handler));
    } else {
        handlers[irq].func = handler;
        handlers[irq].data = data;
        cpu_excep::register_handler(irq, default_handler);
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
    if (irq >= 16) {
        unsigned ext = irq - 16;
        reg(NVIC_ISER + (ext / 32) * 4) = 1U << (ext % 32);
    }
}

void nvic::disable_irq(unsigned irq) {
    if (irq >= 16) {
        unsigned ext = irq - 16;
        reg(NVIC_ICER + (ext / 32) * 4) = 1U << (ext % 32);
    }
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
    default:
        if (irq >= 16 && irq < handlers.size()) {
            unsigned ext = irq - 16;
            reg(NVIC_ISPR + (ext / 32) * 4) = 1U << (ext % 32);
        } else {
            throw exception(lib::fmt::sprint("invalid irq number {}", irq));
        }
        break;
    }
}

void nvic::send_ipi(ipi_target target, unsigned irq) {
    if (target == ipi_target::SELF)
        send_ipi(0x1, irq);
}

}  // namespace device
