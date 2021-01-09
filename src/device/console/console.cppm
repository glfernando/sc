/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module device.console;

import device;
import std.string;

export namespace device {

class console : public device {
 public:
    constexpr static class_type dev_type = class_type::CONSOLE;
    class_type type() const override final { return console::dev_type; }

    console(std::string const& name) : device(name) {}

    // console interface
    virtual void putc(int c, bool wait = true) = 0;
    virtual int getc(bool wait = true) = 0;
};

// console device concept
// This can be use to create generic code that does not use vtable

template <typename T>
concept Console = requires(T t, int c) {
    c = t.getc();
    t.putc(c);
};

}  // namespace device
