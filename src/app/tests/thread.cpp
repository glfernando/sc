/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

#include <test.h>

import core.thread;
import lib.time;
import lib.timestamp;

using core::thread::sleep;
using core::thread::thread_t;

using namespace lib;
using namespace lib::time;

TEST(thread, schedule) {
    bool good = false;
    thread_t t{"test-sched", [](void* data) { *static_cast<bool*>(data) = true; }, &good};
    core::thread::schedule();
    EXPECT(good);
}

TEST(thread, sleep) {
    auto t0 = timestamp::ms();
    sleep(100ms);
    auto delta = timestamp::ms() - t0;
    EXPECT(delta >= 100);
}

TEST(thread, join) {
    bool good = false;
    thread_t t(
        "join",
        [](void* data) {
            sleep(10ms);
            *static_cast<bool*>(data) = true;
        },
        &good);
    t.join();
    EXPECT(good);
}
