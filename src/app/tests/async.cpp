/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

#include <test.h>

import lib.async;
import core.thread;
import lib.cpu;

using lib::async;

int square(int num) {
    return num * num;
}

TEST(async, call) {
    async a1{square, 10};
    async a2{square, 20};

    auto r1 = a1.wait_for_result();
    auto r2 = a2.wait_for_result();

    EXPECT(r1 == 100)
    EXPECT(r2 == 400)
}

TEST(async, affinity) {
    for (unsigned cpu = 0; cpu < core::thread::core_num; ++cpu) {
        async a{1u << cpu, [] { return lib::cpu::id(); }};
        EXPECT(cpu == a.wait_for_result());
    }
}
