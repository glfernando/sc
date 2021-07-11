/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

import soc.rp2040.pwm;
import lib.fmt;
import lib.time;
import lib.servo;
import core.thread;

using lib::fmt::println;
using lib::servo;
using core::thread::sleep;

using namespace lib::time;

constexpr unsigned SERVIO_GPIO = 2;

int main() {
    println("Servo test");

    soc::rp2040::pwm pwm("pwm-servo", SERVIO_GPIO);

    // servo requires adjustment to reach 180 degrees
    servo servo(pwm, 600us, 2400us);

    println("set to 0 position");
    servo.start(0);
    sleep(5s);

    println("rotate 180 degrees slowly");
    for (int i = 0; i <= 180; ++i) {
        sleep(25ms);
        servo.position(i);
    }

    return 0;
}
