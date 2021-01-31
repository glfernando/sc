/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>
#include <stdio.h>

export module device.timer.arm;

export import device.timer;
import device;
import device.intc;

import lib.time;
import std.string;
import lib.lock;
import lib.fmt;

using sc::lib::lock_for;
using sc::lib::lock_irqsafe;
using sc::lib::slock;
using sc::lib::fmt::println;
using sc::lib::time::now;
using sc::lib::time::time_us_t;
using std::string;

namespace device {

namespace {

struct event_arm : public timer::event {
    event_arm(timer::callback cb, void* data) : cb(cb), data(data) {}

    event_arm* next = nullptr;
    event_arm* prev = nullptr;
    timer::callback cb;
    void* data;
    time_us_t period = 0;
    uint64_t exp;
    bool cancelled = false;
    bool is_periodic = false;

    event_arm* operator++() { return next; }

    event_arm* operator*() { return this; }
};

}  // namespace

}  // namespace device

export namespace device {

class timer_arm : public timer {
 public:
    struct platform_data {
        unsigned irq;
    };

    timer_arm(string const& name, platform_data const& pdata) : timer(name), irq(pdata.irq) {}

    inline void init() override;
    inline event* create(enum type, callback cb, void* data) override;
    inline void set(event* e, time_us_t period) override;
    inline void cancel(event* e) override;
    inline void destroy(event* e) override;

 private:
    struct timer_event_queue {
        timer_event_queue() : list(nullptr) {}
        bool empty() const { return list == nullptr; }
        event_arm* front() { return list; }
        event_arm* pop() {
            if (list == nullptr)
                return nullptr;
            event_arm* e = list;
            list = e->next;
            e->next = e->prev = nullptr;
            return e;
        }
        // we can only insert in an order way
        void insert(event_arm* e) {
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
        void dump() {
            println("dump");
            for (auto tmp = list; tmp; tmp = tmp->next)
                println("exp:{}", tmp->exp);
        }
        void remove(event_arm* e) {
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
        event_arm* list;
    };
    unsigned irq;
    void isr();
    lock_irqsafe lock;
    // TODO: consider standard container once they are available
    timer_event_queue queue;
};

}  // namespace device

namespace device {

void timer_arm::init() {
    auto intc = manager::find<::device::intc>();
    intc->request_irq(
        irq, intc::FLAG_START_ENABLED,
        [](unsigned, void* data) { reinterpret_cast<timer_arm*>(data)->isr(); }, this);

    //sysreg_write(cntp_cval_el0, -1L);
    //sysreg_write(cntp_ctl_el0, 1L);
}

timer::event* timer_arm::create(enum type type, callback cb, void* data) {
    auto e = new event_arm(cb, data);
    e->is_periodic = type == timer::type::PERIODIC;
    return e;
}

void timer_arm::set(timer::event* event, time_us_t period) {
    auto e = static_cast<event_arm*>(event);
    // fill event data
    e->period = period;
    auto to = now() + period;
    e->exp = to.ticks();

    slock guard(lock);
    queue.insert(e);
    //if (e == queue.front())
        //sysreg_write(cntp_cval_el0, e->exp);
}

void timer_arm::cancel(timer::event* event) {
    auto e = static_cast<event_arm*>(event);
    slock guard(lock);
    e->cancelled = true;
    queue.remove(e);
}

void timer_arm::destroy(timer::event* event) {
    cancel(event);
    delete static_cast<event_arm*>(event);
}

void timer_arm::isr() {
    auto curr = now().ticks();
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
                e->exp += e->period.ticks();
            } while (e->exp <= curr);
            // insert it back
            queue.insert(e);
        }
    }
    //auto e = queue.front();
    //if (e)
        //sysreg_write(cntp_cval_el0, e->exp);
    //else
        //sysreg_write(cntp_cval_el0, -1L);
    lock.release();
}

}  // namespace device
