/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module soc.rp2040.i2c;

#include <stddef.h>
#include <stdint.h>

export import device.i2c;

import std.string;
import lib.fmt;
import lib.reg;

using lib::fmt::println;
using lib::reg::reg32;
using std::string;

namespace {

// clang-format off
enum regs : uint32_t {
    CON         = 0x00,
    TAR         = 0x04,
    DATA_CMD    = 0x10,
    FS_SCL_HCNT = 0x1c,
    FS_SCL_LCNT = 0x20,
    ENABLE      = 0x6c,
    STATUS      = 0x70,
    TXFLR       = 0x74,
    RXFLR       = 0x78,
    DMA_CR      = 0x88,
    FS_SPKLEN   = 0xa0,
};
// clang-format on

};  // namespace

export namespace soc::rp2040 {

class i2c : public device::i2c {
 public:
    struct platform_data {
        unsigned base;
        unsigned freq;
        mode mode;
    };

    i2c(std::string const& name, platform_data const& pdata)
        : ::device::i2c(name),
          base(pdata.base),
          freq(pdata.freq),
          mode(pdata.mode),
          restart(false) {}

    inline void init() override;
    inline void set_mode(mode) override {}
    inline void read(uint8_t addr, uint8_t* buffer, size_t size, bool nostop = false) override;
    inline void write(uint8_t addr, uint8_t const* buffer, size_t size,
                      bool nostop = false) override;

 private:
    volatile uint32_t& reg(uint32_t offset) { return reg32(base + offset); }

    unsigned base;
    unsigned freq;
    mode mode;
    bool restart;
};

}  // namespace soc::rp2040

namespace soc::rp2040 {

void i2c::init() {
    reg(DMA_CR) = 0x3;

    unsigned baudrate;
    switch (mode) {
    case i2c::mode::STANDARD:
        baudrate = 100'000;
        break;
    default:  // max support is FAST
        baudrate = 400'000;
    }

    unsigned period = (freq + baudrate / 2) / baudrate;
    unsigned hcnt = period * 3 / 5;
    unsigned lcnt = period - hcnt;

    reg(ENABLE) = 0;

    reg(FS_SCL_HCNT) = hcnt;
    reg(FS_SCL_LCNT) = lcnt;
    reg(FS_SPKLEN) = lcnt < 16 ? 1 : lcnt / 16;

    reg(ENABLE) = 1;
}

void i2c::read(uint8_t addr, uint8_t* buffer, size_t size, bool nostop) {
    reg(ENABLE) = 0;
    reg(TAR) = addr;
    reg(ENABLE) = 1;

    for (size_t i = 0; i < size; ++i) {
        while (!(reg(STATUS) & (1 << 1))) {}

        uint32_t cmd = 1 << 8;

        if (i == 0 && restart) {
            cmd |= 1 << 10;
        }

        if (i == (size - 1) && !nostop) {
            cmd |= 1 << 9;
        }

        reg(DATA_CMD) = cmd;

        while (!reg(RXFLR)) {}

        *buffer++ = reg(DATA_CMD) & 0xff;
    }

    restart = nostop;
}

void i2c::write(uint8_t addr, uint8_t const* buffer, size_t size, bool nostop) {
    reg(ENABLE) = 0;
    reg(TAR) = addr;
    reg(ENABLE) = 1;

    for (size_t i = 0; i < size; ++i) {
        uint32_t cmd = *buffer++;

        if (i == 0 && restart) {
            cmd |= 1 << 10;
        }

        if (i == (size - 1) && !nostop) {
            cmd |= 1 << 9;
        }

        reg(DATA_CMD) = cmd;
        while (!(reg(STATUS) & (1 << 2))) {}
    }

    restart = nostop;
}

}  // namespace soc::rp2040
