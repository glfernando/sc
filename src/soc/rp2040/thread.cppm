/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module soc.thread;

import soc.rp2040.mailbox;

export namespace soc::thread {

void kick() {
    soc::rp2040::mailbox::write(0x0);
}

}  // namespace soc::thread
