/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

#include <test.h>

import lib.async;
import core.thread;

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
