/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stddef.h>
#include <stdint.h>

export module core.thread;
import core.thread.arch;
import device.timer;

import std.string;
import std.memory;
import std.vector;
import lib.exception;
import lib.fmt;
import device;
import lib.lock;
import lib.time;
import lib.elist;
import lib.equeue;

using lib::equeue;
using lib::exception;
using lib::lock;
using lib::lock_for;
using lib::lock_irqsafe;
using lib::slock;
using lib::fmt::println;
using std::string;
using std::unique_ptr;
using std::vector;

using namespace lib::time;

constexpr size_t THREAD_STACK_SIZE = 1024 * 32;
using entry_t = void (*)(void*);

export namespace core::thread {

enum class state {
    READY,
    BLOCKED,
    ASLEEP,
    RUNNING,
    DONE,
    DEAD,
};

class thread_t : public thread_arch, public lib::elist_node {
 public:
    static void thread_entry(thread_t* self);

    thread_t(string const& name, entry_t entry, void* arg = nullptr,
             size_t stack_size_ = THREAD_STACK_SIZE);

    ~thread_t() {
        // only propagate exception when join is called directly
        try {
            join();
        } catch (...) {}
    }

    void join();

    string name;
    state state;

 private:
    entry_t entry;
    void* arg;
    unique_ptr<uint8_t> stack;
    size_t stack_size;
    exception excep = {"", 0};
    vector<thread_t*> done_wait_list;

    thread_t() {}
    friend void init();
    friend void schedule();
};

}  // namespace core::thread

// local variables
namespace core::thread {

static lock_irqsafe thread_lock;
static equeue<thread_t> ready_queue;
static thread_t* idle_thread;
static device::timer* dev;

}  // namespace core::thread

export namespace core::thread {
thread_t* current() {
    return reinterpret_cast<thread_t*>(thread_current_addr());
}

void schedule() {
    slock guard(thread_lock);

    auto t = current();

    switch (t->state) {
    case state::DONE:
        t->state = state::DEAD;
        for (auto tmp : t->done_wait_list) {
            tmp->state = state::READY;
            ready_queue.push(tmp);
        }
        break;
    default:;
    }

    thread_t* new_t;
    if (ready_queue.empty()) {
        if (t == idle_thread || t->state == state::RUNNING) {
            // println("empty continue currning");
            return;
        }
        // switch to idle thread
        new_t = idle_thread;
    } else {
        new_t = ready_queue.pop();
    }

    // println("{} switching to {}", new_t, new_t->name);

    if (t != idle_thread && t->state == state::RUNNING) {
        t->state = state::READY;
        ready_queue.push(t);
    }

    new_t->state = state::RUNNING;

    thread_current_addr(reinterpret_cast<uintptr_t>(new_t));
    switch_context(new_t, t);
}

void set_ready(thread_t* t) {
    slock guard(thread_lock);
    if (t->state != state::READY) {
        t->state = state::READY;
        ready_queue.push(t);
    }
}

void sleep_timer_cb(void* data) {
    thread_t* t = static_cast<thread_t*>(data);
    lock_for(thread_lock, [&] {
        t->state = state::READY;
        ready_queue.push(t);
    });
}

void sleep(time_ms_t period) {
    auto t = reinterpret_cast<thread_t*>(thread_current_addr());
    auto e = dev->create(device::timer::type::ONE_SHOT, sleep_timer_cb, t);
    lock_for(thread_lock, [&] {
        t->state = state::ASLEEP;
        dev->set(e, period);
    });
    while (t->state == state::ASLEEP)
        schedule();
}

void thread_idle(void*) {
    for (;;) {
        arch_idle();
        schedule();
    }
}

void init() {
    // make a thread for the current execution
    auto ti = new thread_t();
    ti->name = "init";
    ti->entry = nullptr;
    ti->arg = nullptr;
    ti->state = state::RUNNING;

    // tell the system it is the current thread
    thread_current_addr(reinterpret_cast<uintptr_t>(ti));

    // now let's create a idle thread
    idle_thread = new thread_t();
    idle_thread->name = "idle";
    idle_thread->entry = thread_idle;
    idle_thread->arg = nullptr;
    idle_thread->state = state::READY;
    idle_thread->stack.reset(new uint8_t[IDLE_THREAD_STACK_SIZE]);
    idle_thread->stack_size = IDLE_THREAD_STACK_SIZE;
    auto sp = reinterpret_cast<uintptr_t>(idle_thread->stack.get()) + idle_thread->stack_size;
    auto pc = reinterpret_cast<uintptr_t>(&idle_thread->thread_entry);
    auto arg = reinterpret_cast<unsigned long>(idle_thread);
    idle_thread->init_context(pc, arg, sp);

    dev = device::manager::find<device::timer>();
}

}  // namespace core::thread

// thread_t implementation
namespace core::thread {

void thread_t::thread_entry(thread_t* self) {
    // context switch is always done with thread_lock held, so manually release it the first
    // time we enter to the thread.
    thread_lock.release();
    try {
        self->entry(self->arg);
    } catch (exception& e) {
        println("thread {} caused exception ({})", self->name, e.msg());
        self->excep = e;
    } catch (...) { self->excep = exception("unknown"); }
    self->state = state::DONE;
    schedule();
}

thread_t::thread_t(string const& name, entry_t entry, void* arg, size_t stack_size_)
    : name(name), entry(entry), arg(arg), stack(new uint8_t[stack_size_]), stack_size(stack_size_) {
    auto sp = reinterpret_cast<uintptr_t>(stack.get()) + stack_size;
    auto pc = reinterpret_cast<uintptr_t>(&thread_entry);
    auto self_ptr = reinterpret_cast<unsigned long>(this);
    init_context(pc, self_ptr, sp);

    lock_for(thread_lock, [this] {
        state = state::READY;
        ready_queue.push(this);
    });
}

void thread_t::join() {
    if (state == state::DEAD)
        return;

    auto t = current();
    if (t == this) {
        println("cannot call join from the same thread");
        return;
    }

    lock_for(thread_lock, [&] {
        t->state = state::BLOCKED;
        done_wait_list.push_back(t);
    });

    while (state != state::DEAD) {
        schedule();
    }

    if (excep.error()) {
        throw excep;
    }
}

}  // namespace core::thread
