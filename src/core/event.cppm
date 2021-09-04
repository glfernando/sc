/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module core.event;

#include <errcodes.h>

import core.thread;
import lib.fmt;
import lib.equeue;
import lib.lock;
import lib.time;
import lib.timer;
import lib.exception;

using core::thread::thread_t;
using lib::equeue;
using lib::lock;
using lib::slock;
using lib::fmt::println;

using namespace lib;
using namespace lib::time;
using lib::exception;
using lib::timer;

constexpr unsigned SIGNALED_ALL = -1;

export namespace core {

class event {
 public:
    event() : pending_count(0) {}
    void wait_for_signal(time_us_t timeout = time::INFINITE);
    void signal(bool all = false);

 private:
    lock lock;
    unsigned pending_count;
    equeue<thread_t> wait_list;
};

}  // namespace core

namespace core {

void event::wait_for_signal(time_us_t timeout) {
    {
        slock_irqsafe guard(lock);
        if (pending_count == SIGNALED_ALL)
            return;
        if (pending_count) {
            pending_count--;
            return;
        }

        auto t = thread::current();
        wait_list.push(t);
        t->state = thread::state::BLOCKED;
    }

    bool to_expeired = false;
    timer timer{timer::type::ONE_SHOT};
    auto t = thread::current();
    if (timeout != time::INFINITE) {
        timer.start(
            [this, &to_expeired, &t] {
                to_expeired = true;
                lock_irqsafe_for(lock, [&] {
                    // move thread from wait list to ready state
                    wait_list.remove(t);
                    thread::set_ready(t);
                });
                thread::schedule();
            },
            timeout);
    }

    while (t->state == thread::state::BLOCKED) {
        thread::schedule();
    }
    if (to_expeired) {
        throw exception("event timeout", ERR_TIMED_OUT);
    }
}

void event::signal(bool all) {
    slock_irqsafe guard(lock);
    if (all) {
        pending_count = SIGNALED_ALL;
        while (!wait_list.empty()) {
            thread::set_ready(wait_list.pop());
        }
        return;
    }

    if (!wait_list.empty()) {
        auto t = wait_list.pop();
        thread::set_ready(t);
        return;
    }

    pending_count++;
}

}  // namespace core
