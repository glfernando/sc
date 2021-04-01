/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

#include <test.h>

import lib.timer;
import lib.time;

using lib::timer;

using namespace lib;
using namespace lib::time;

TEST(timer, oneshot) {
    timer t(timer::type::ONE_SHOT);

    int counter = 0;
    t.start([&counter] { counter++; }, 1ms);
    delay(5ms);
    // make sure callback was executed only once
    EXPECT(counter == 1);
}

TEST(timer, periodic) {
    int counter;
    {
        timer t;
        counter = 0;
        t.start([&counter] { counter++; }, 5ms);
        delay(50ms + 2ms);
    }
    EXPECT(counter == 10);
}

TEST(timer, stop) {
    timer t1;
    int counter = 0;
    t1.start(
        [&counter, &t1]() mutable {
            if (++counter == 5)
                t1.stop();
        },
        1ms);
    delay(10ms);
    EXPECT(counter == 5);

    // oneshot timer, stop before it is fired
    timer t2(timer::type::ONE_SHOT);
    counter = 0;
    t2.start([&counter] { counter++; }, 10ms);
    delay(5ms);
    t2.stop();
    delay(10ms);
    EXPECT(counter == 0)
}

TEST(timer, reschedule) {
    timer t(timer::type::ONE_SHOT);

    int counter = 0;
    t.start([&counter, &t] { t.start(1ms + ++counter); }, 1ms);
    delay(18ms);
    t.stop();
    EXPECT(counter == 5)
}
