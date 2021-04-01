/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>

export module device.intc.gic;

export import device.intc;
import core.cpu;
import std.string;
import lib.reg;
import lib.fmt;
import lib.exception;

using lib::exception;
using lib::fmt::println;
using lib::fmt::sprint;
using lib::reg::reg32;
using std::string;

export namespace device {

class gic : public intc {
 public:
    struct platform_data {
        uintptr_t dbase;
        uintptr_t cbase;
    };

    gic(string const& name, platform_data const& pdata)
        : intc(name), dbase(pdata.dbase), cbase(pdata.cbase) {}

    inline void init() override;
    inline void request_irq(unsigned irq, unsigned flags, handler handler, void* data) override;
    inline void free_irq(unsigned irq) override;
    inline void enable_irq(unsigned irq) override;
    inline void disable_irq(unsigned irq) override;
    inline void send_ipi(unsigned cpu_mask, unsigned irq) override;
    inline void send_ipi(ipi_target target, unsigned irq) override;

 private:
    volatile uint32_t& dreg(uint32_t offset) { return reg32(dbase + offset); }
    volatile uint32_t& creg(uint32_t offset) { return reg32(cbase + offset); }
    void isr();

    uintptr_t dbase;
    uintptr_t cbase;
    unsigned irq_num;
    struct handler_info {
        handler handler;
        void* data;
    };
    handler_info* handlers;
};

}  // namespace device

namespace device {

// clang-format off
enum dreg_reg_offset : uint32_t {
    GICD_CTLR       = 0x0000,
    GICD_TYPER      = 0x0004,
    GICD_ISENABLER  = 0x0100,
    GICD_ICENABLER  = 0x0180,
    GICD_ICPENDR    = 0x0280,
    GICD_SGIR       = 0x0f00,
};

enum GICD_CTLR_bits : uint32_t {
    GICD_CTLR_EN_GRP0    = 1 << 0,
    GICD_CTLR_EN_GRP1_NS = 1 << 1,
};

enum cpu_intf_reg_offset : uint32_t {
    GICC_CTLR   = 0x00,
    GICC_PMR    = 0x04,
    GICC_IAR    = 0x0c,
    GICC_EOIR   = 0x10,
};

enum GICC_CTLR_bits : uint32_t {
    GICC_CTLR_ENABLE    = 1 << 0,
};
// clang-format on

constexpr unsigned GIC_SPURIOUS_INT = 1023;
constexpr unsigned GIC_SPI_START = 32;
constexpr unsigned GIC_SGI_MAX = 16;

void gic::init() {
    auto v = dreg(GICD_TYPER);
    irq_num = ((v & 0x1f) + 1) * 32;
    println("number of irq lines {}", irq_num);

    if (irq_num < GIC_SPI_START)
        throw exception(sprint("invalid number of interrupts {}", irq_num));

    handlers = new handler_info[irq_num - GIC_SPI_START];
    core::cpu::register_irq_handler([](int, void* obj) { static_cast<gic*>(obj)->isr(); }, this);

    // disable and clear any pending interrupt
    for (unsigned i = 32; i < irq_num; i += 32) {
        dreg(GICD_ICENABLER + (i / 32) * 4) = ~0;
        dreg(GICD_ICPENDR + (i / 32) * 4) = ~0;
    }

    // enable gic distributor
    dreg(GICD_CTLR) |= GICD_CTLR_EN_GRP1_NS | GICD_CTLR_EN_GRP0;

    // core enable
    creg(GICC_CTLR) = GICC_CTLR_ENABLE;
    // allow all priorities
    creg(GICC_PMR) = 0xff;

    asm volatile("msr daifclr, #3");
}

void gic::request_irq(unsigned irq, unsigned flags, handler handler, void* data) {
    if (irq >= irq_num)
        throw exception("invalid requested irq number");

    handlers[irq].handler = handler;
    handlers[irq].data = data;

    if (flags & FLAG_START_ENABLED)
        enable_irq(irq);
}

void gic::free_irq(unsigned irq) {
    // make sure it is disabled
    disable_irq(irq);
    handlers[irq].handler = nullptr;
    handlers[irq].data = nullptr;
}

void gic::enable_irq(unsigned irq) {
    if (irq >= irq_num)
        throw exception("invalid irq number");
    uint32_t bit = irq % 32;
    uintptr_t offset = GICD_ISENABLER + (irq / 32) * 4;
    dreg(offset) = 1 << bit;
}

void gic::disable_irq(unsigned irq) {
    if (irq >= irq_num)
        throw exception("invalid irq number");
    uint32_t bit = irq % 32;
    uintptr_t offset = GICD_ICENABLER + (irq / 32) * 4;
    dreg(offset) = 1 << bit;
}

void gic::isr() {
    uint32_t v = creg(GICC_IAR);
    unsigned irq = v & 0x3ff;

    do {
        if (irq == GIC_SPURIOUS_INT) {
            println("spurious interrupt\n");
            break;
        }

        if (irq >= irq_num) {
            println("invalid irq num {}", irq);
            break;
        }

        auto& handler = handlers[irq];
        if (!handler.handler) {
            println("no handler for irq {}", irq);
            break;
        }

        handler.handler(irq, handler.data);
    } while (0);

    creg(GICC_EOIR) = v;
}

void gic::send_ipi(unsigned cpu_mask, unsigned irq) {
    if (irq >= GIC_SGI_MAX) {
        throw exception("invalid irq");
    }

    if (!cpu_mask || (cpu_mask >> 8)) {
        throw exception(sprint("invalid cpu irq mask {:#x}", cpu_mask));
    }

    uint32_t v = irq | (cpu_mask << 16);
    dreg(GICD_SGIR) = v;
}

void gic::send_ipi(ipi_target target, unsigned irq) {
    if (target == ipi_target::ALL) {
        return send_ipi(0xff, irq);
    }

    if (irq >= GIC_SGI_MAX) {
        throw exception("invalid irq");
    }
    uint32_t v = irq;
    v |= (target == ipi_target::SELF ? 2 : 1) << 24;
    dreg(GICD_SGIR) = v;
}

}  // namespace device
