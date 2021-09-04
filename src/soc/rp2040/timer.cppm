/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

// TODO: finish implemenation and move under src/soc/rp2040/

module;

#include <stdint.h>

export module device.timer.rp2040;
export import device.timer;

import device.intc;
import std.string;
import lib.time;
import lib.fmt;
import lib.reg;
import lib.lock;

using lib::lock;
using lib::slock_irqsafe;
using lib::fmt::println;
using lib::reg::reg32;
using lib::time::time_us_t;
using std::string;

namespace device {

namespace {

// clang-format off
enum regs : uint32_t {
    TIMERHR = 0x08,
    TIMERLR = 0x0c,
    ALARM0  = 0x10,
    INTE    = 0x38,
};
// clang-format on

struct event_impl : public timer::event {
    event_impl(timer::callback cb, void* data) : cb(cb), data(data) {}

    event_impl* next = nullptr;
    event_impl* prev = nullptr;
    timer::callback cb;
    void* data;
    time_us_t period = 0;
    uint64_t exp;
    bool cancelled = false;
    bool is_periodic = false;

    event_impl* operator++() { return next; }

    event_impl* operator*() { return this; }
};

}  // namespace

}  // namespace device

export namespace device {

class timer_rp2040 : public timer {
 public:
    struct platform_data {
        uintptr_t base;
        unsigned irq;
    };

    timer_rp2040(string const& name, platform_data const& pdata)
        : timer(name), base(pdata.base), irq(pdata.irq) {}

    inline void init() override;
    inline event* create(enum type, callback, void*) override;
    inline void set(event*, time_us_t) override;
    inline void cancel(event*) override;
    inline void destroy(event*) override;

 private:
    struct timer_event_queue {
        timer_event_queue() : list(nullptr) {}
        bool empty() const { return list == nullptr; }
        event_impl* front() { return list; }
        event_impl* pop() {
            if (list == nullptr)
                return nullptr;
            event_impl* e = list;
            list = e->next;
            e->next = e->prev = nullptr;
            return e;
        }
        // we can only insert in an order way
        void insert(event_impl* e) {
            if (e->next || e->prev) {
                println("invalid insert");
            }
            if (list == nullptr) {
                list = e;
            } else {
                for (auto tmp = list; tmp; tmp = tmp->next) {
                    if (e->exp < tmp->exp) {
                        e->prev = tmp->prev;
                        tmp->prev = e;
                        if (e->prev)
                            e->prev->next = e;
                        e->next = tmp;
                        if (list == tmp)
                            list = e;
                    } else if (tmp->next == nullptr) {
                        // tmp is the last one, so insert e after it
                        tmp->next = e;
                        e->prev = tmp;
                        break;
                    }
                }
            }
        }
        void remove(event_impl* e) {
            if (e == list) {
                pop();
            } else {
                if (e->next)
                    e->next->prev = e->prev;
                if (e->prev)
                    e->prev->next = e->next;
                e->next = e->prev = nullptr;
            }
        }
        event_impl* list;
    };

    volatile uint32_t& reg(uint32_t offset) { return reg32(base + offset); }
    void isr();

    uintptr_t base;
    unsigned irq;
    lock lock;
    // TODO: consider standard container once they are available
    timer_event_queue queue;
};

timer::event* timer_rp2040::create(enum type type, callback cb, void* data) {
    auto e = new event_impl(cb, data);
    e->is_periodic = type == timer::type::PERIODIC;
    return e;
}

void timer_rp2040::set(event* event, time_us_t period) {
    auto e = static_cast<event_impl*>(event);
    // fill event data
    e->period = period;
    uint64_t now = reg(TIMERLR) | static_cast<uint64_t>(reg(TIMERHR)) << 32;
    e->exp = now + period.get_val();

    slock_irqsafe guard(lock);
    queue.insert(e);
    if (e == queue.front())
        reg(ALARM0) = e->exp;
}

void timer_rp2040::isr() {
    reg(0x34) = 0x1;

    uint64_t curr = reg(TIMERLR) | static_cast<uint64_t>(reg(TIMERHR)) << 32;

    lock.acquire();
    // queue.dump();
    while (auto e = queue.front()) {
        if (e->exp > curr)
            break;
        e = queue.pop();
        // release lock so that callback can modify timer, e.g. cancer or reschedule
        lock.release();
        try {
            e->cb(e->data);
        } catch (...) { println("timer callback exception"); }
        lock.acquire();
        if (!e->cancelled && e->is_periodic) {
            do {
                e->exp += e->period.get_val();
            } while (e->exp <= curr);
            // insert it back
            queue.insert(e);
        }
    }
    auto e = queue.front();
    if (e)
        reg(ALARM0) = e->exp;
    lock.release();
}

void timer_rp2040::cancel(timer::event* event) {
    auto e = static_cast<event_impl*>(event);
    slock_irqsafe guard(lock);
    e->cancelled = true;
    queue.remove(e);
}

void timer_rp2040::destroy(timer::event* event) {
    cancel(event);
    delete static_cast<event_impl*>(event);
}

void timer_rp2040::init() {
    auto intc = manager::find<::device::intc>();
    intc->request_irq(
        irq, intc::FLAG_START_ENABLED,
        [](unsigned, void* data) { reinterpret_cast<timer_rp2040*>(data)->isr(); }, this);

    // enable ALARM0 interrupt
    reg(INTE) = 1;
}

}  // namespace device
