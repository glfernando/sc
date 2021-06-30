/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

#include <test.h>

import lib.async;
import lib.time;
import core.event;
import core.thread;
import lib.exception;

using lib::async;
using namespace lib;
using namespace lib::time;
using core::event;
using lib::exception;

static int result;

void event_signal(event* e) {
    core::thread::sleep(200ms);
    result *= 6;
    e->signal();
    core::thread::sleep(100ms);
    e->signal();
}

void event_wait(event* e) {
    e->wait_for_signal();
    result += 1;
}

TEST(event, wait) {
    event e;
    async a1{"signal", event_signal, &e};
    async a2{"wait", event_wait, &e};
    result = 3;
    e.wait_for_signal();
    a2.wait_for_result();
    EXPECT(result = 19)
}
