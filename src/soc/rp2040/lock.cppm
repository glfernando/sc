/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module lib.lock.arch;

import soc.rp2040.hwspinlock;

using soc::rp2040::hwspinlock;

static unsigned hw_index;

export namespace lib {

class lock {
 public:
    lock() : hwlock(hw_index++), val(0) {}

    [[gnu::always_inline]] void acquire();
    [[gnu::always_inline]] void release();

 public:
    hwspinlock hwlock;
    unsigned val;
};

}  // namespace lib

namespace lib {

void lock::acquire() {
    unsigned old;
    do {
        hwlock.acquire();
        old = val;
        if (val == 0) {
            val = 1;
        }
        hwlock.release();
    } while (old);
}

void lock::release() {
    val = 0;
}

}  // namespace lib
