/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module device.console.uart;

import device;
import device.uart;
import device.console;
import std.string;

export namespace device {

class uart_console : public console {
 public:
    uart_console(std::string const& name, uart& uart) : console(name), uart(uart) {}

    void putc(int c, bool wait = true) override { uart.putc(c, wait); }
    int getc(bool wait = true) override { return uart.getc(wait); }

 private:
    uart& uart;
};

}  // namespace device
