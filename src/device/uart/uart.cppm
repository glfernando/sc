/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module device.uart;

export import device;
import std.string;

export namespace device {

class uart : public device {
 public:
    constexpr static class_type dev_type = class_type::UART;
    class_type type() const override final { return uart::dev_type; }

    uart(std::string const& name) : device(name) {}

    // uart interface
    virtual void baudrate(unsigned baud) = 0;
    virtual void putc(int c, bool wait = true) = 0;
    virtual int getc(bool wait = true) = 0;
    virtual void flush() = 0;
};

// uart device concept
// This can be use to create generic code that does not use vtable

template <typename T>
concept Uart = requires(T t, unsigned baud, int c)
{
    t.baudrate(baud);
    c = t.getc();
    t.putc(c);
    t.flush();
};

};  // namespace device
