/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module device.i2c;

#include <stddef.h>
#include <stdint.h>

export import device;
import std.string;

export namespace device {

class i2c : public device {
 public:
    constexpr static class_type dev_type = class_type::I2C;
    class_type type() const override final { return i2c::dev_type; }

    enum class mode {
        STANDARD,
        FAST,
        FAST_PLUS,
        HIGH_SPEED,
        ULTRA,
    };

    i2c(std::string const& name) : device(name) {}

    virtual void set_mode(mode mode) = 0;
    // TODO: change buffer and size to a std::span or lib::buffer etc. Also add timeouts
    virtual void read(uint8_t addr, uint8_t* buffer, size_t size, bool nostop = false) = 0;
    virtual void write(uint8_t addr, uint8_t const* buffer, size_t size, bool nostop = false) = 0;
    uint8_t reg(uint8_t addr, uint8_t offset) {
        write(addr, &offset, 1, true);
        uint8_t val;
        read(addr, &val, 1);
        return val;
    }
    void reg(uint8_t addr, uint8_t offset, uint8_t data) {
        uint8_t buf[2] = {offset, data};
        write(addr, buf, 2);
    }
    // TODO: add template function to read data from a register for any size
};

}  // namespace device
