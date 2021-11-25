/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

#include <test.h>

import core.thread;
import lib.time;
import lib.timestamp;
import lib.lock;
import lib.cpu;

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

TEST(thread, busy) {
    thread_t t1(
        "busy-t1", [](void*) { delay(1s); }, nullptr);

    thread_t t2(
        "busy-t2", [](void*) { delay(1s); }, nullptr);
    t1.join();
    t2.join();
}

static int counter;
static lock lock0;

TEST(thread, spinlock) {
    counter = 0;
    thread_t t1(
        "spin-t1",
        [](void*) {
            for (int i = 0; i < 100; ++i) {
                lock0.acquire();
                counter++;
                lock0.release();
            }
        },
        nullptr);

    for (int i = 0; i < 100; ++i) {
        lock0.acquire();
        counter++;
        lock0.release();
    }
    t1.join();
    EXPECT(counter == 200);
}

TEST(thread, affinity) {
    // schedule a thread on each core
    unsigned tcpu = -1;
    for (unsigned cpu = 0; cpu < core::thread::core_num; ++cpu) {
        thread_t t(
            "aff-thread",
            [](void* data) {
                unsigned& cpu = *reinterpret_cast<unsigned*>(data);
                cpu = lib::cpu::id();
            },
            &tcpu, 1 << cpu);

        t.join();

        EXPECT(cpu == tcpu);
    }
}
