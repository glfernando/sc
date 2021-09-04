/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module lib.lock;

export import lib.lock.arch;

import lib.cpu;

export namespace lib {

template <typename T>
concept Lockable = requires(T t) {
    t.acquire();
    t.release();
};

// scoped (or secure) lock
//  Use creates a wrapper around a lockable object and it makes sure lock is released when exiting
// the scope. This is secure even under an exception
template <Lockable T>
class slock {
 public:
    slock(T& t) : lock(t) { lock.acquire(); }
    ~slock() { lock.release(); }

 private:
    T& lock;
};

template <Lockable T>
class slock_irqsafe {
 public:
    slock_irqsafe(T& t) : lock(t) {
        flags = cpu::save_and_disable_irq();
        lock.acquire();
    }
    ~slock_irqsafe() {
        lock.release();
        cpu::restore_irq(flags);
    }

 private:
    unsigned long flags;
    T& lock;
};

// This helper function will hold @L lock for the duration of the callable object @F. It is secure
// even if code inside @F can throw exceptions.
// This helper is most likely to be used with a lambda which includes the code we want to protect.
// Compiler most likely will inline everything having not extra cost
template <Lockable L, typename F>
void lock_for(L& lock, F&& func) {
    slock sl(lock);
    func();
}

template <Lockable L, typename F>
void lock_irqsafe_for(L& lock, F&& func) {
    slock_irqsafe sl(lock);
    func();
}

}  // namespace lib
