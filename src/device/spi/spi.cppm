/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module device.spi;

#include <stddef.h>
#include <stdint.h>

export import device;
import std.string;

export namespace device {

class spi : public device {
 public:
    constexpr static class_type dev_type = class_type::SPI;
    class_type type() const override final { return spi::dev_type; }

    spi(std::string const& name) : device(name) {}

    virtual void transfer(uint8_t* tx, uint8_t* rx, size_t size) = 0;
    // write passed value multiple times, value usually is a inteter of 1/2/4/8 bytes long
    virtual void write_repeat(void* val, size_t val_size, size_t count, bool bigendian = false) = 0;

    // helper functions
    inline void read(uint8_t* buf, size_t size) { transfer(nullptr, buf, size); }
    inline void write(uint8_t* buf, size_t size) { transfer(buf, nullptr, size); }
};

}  // namespace device
