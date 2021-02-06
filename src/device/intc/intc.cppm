/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module device.intc;

export import device;
import std.string;

export namespace device {

class intc : public device {
 public:
    constexpr static class_type dev_type = class_type::INTC;
    class_type type() const override final { return intc::dev_type; }

    intc(std::string const& name) : device(name) {}

    using handler = void (*)(unsigned irq, void* data);
    enum class ipi_target {
        SELF = 0,
        ALL_BUT_ME,
        ALL,
    };

    // non scoped enum and flags parameter is unsigned, so that implementation of INTC can extend
    // flags
    // clang-format off
    enum flags {
        // use bits [1:0] for trigger mode
        FLAG_LEVEL_HIGH     = 0,
        FLAG_LEVEL_LOW      = 1,
        FLAG_EDGE_RISING    = 2,
        FLAG_EDGE_FALLING   = 3,

        // enable interrupt when calling request_irq
        FLAG_START_ENABLED  = 1 << 3,
    };
    // clang-format on

    // uart interface
    virtual void request_irq(unsigned irq, unsigned flags, handler handler, void* data) = 0;
    virtual void free_irq(unsigned irq) = 0;
    virtual void enable_irq(unsigned irq) = 0;
    virtual void disable_irq(unsigned irq) = 0;
    virtual void send_ipi(unsigned cpu_mask, unsigned irq) = 0;
    virtual void send_ipi(ipi_target target, unsigned irq) = 0;
};

template <typename T>
concept Intc = requires(T t, unsigned irq, unsigned flags, unsigned mask, intc::ipi_target target,
                        intc::handler handler) {
    t.request_irq(irq, flags, handler);
    t.free_irq(irq);
    t.enable_irq(irq);
    t.disable_irq(irq);
    t.send_ipi(mask, irq);
    t.send_ipi(target, irq);
};

}  // namespace device
