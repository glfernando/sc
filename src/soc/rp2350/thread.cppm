/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module soc.thread;

export namespace soc::thread {

void kick() {
    asm volatile("sev");
}

}  // namespace soc::thread
