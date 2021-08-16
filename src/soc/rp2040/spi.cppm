/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module soc.rp2040.spi;

#include <endian.h>
#include <stddef.h>
#include <stdint.h>

export import device.spi;

import std.string;
import lib.fmt;
import lib.reg;
import lib.exception;

using lib::exception;
using lib::fmt::println;
using lib::reg::reg32;
using std::string;

namespace {

// clang-format off
enum regs : uint32_t {
    SSPCR0      = 0x00,
    SSPCR1      = 0x04,
    SSPDR       = 0x08,
    SSPSR       = 0x0c,
    SSPCPSR     = 0x10,
    SSPICR      = 0x20,
    SSPDMACR    = 0x24,
};
// clang-format on

};  // namespace

export namespace soc::rp2040 {

class spi : public device::spi {
 public:
    struct platform_data {
        uintptr_t base;
        unsigned freq;
        unsigned baudrate;
    };

    spi(std::string const& name, platform_data const& pdata)
        : ::device::spi(name),
          base(pdata.base),
          freq(pdata.freq),
          baudrate(pdata.baudrate),
          init_done(false) {}

    inline void init() override;
    inline void transfer(uint8_t* tx, uint8_t* rx, size_t size) override;
    inline void write_repeat(void* val, size_t val_size, size_t count,
                             bool bigendian = false) override;

 private:
    volatile uint32_t& reg(uint32_t offset) { return reg32(base + offset); }

    unsigned base;
    unsigned freq;
    unsigned baudrate;
    bool init_done;
};

}  // namespace soc::rp2040

namespace soc::rp2040 {

void spi::init() {
    if (init_done) {
        // TODO: add multithread support
        return;
    }

    // make spi out of reset
    // TODO: move it to SoC init ?
    reg32(0x4000c000) &= ~(0x3 << 16);

    unsigned prescale;
    for (prescale = 2; prescale <= 254; prescale += 2) {
        if (freq < (prescale + 2) * 256 * baudrate)
            break;
    }

    unsigned postdiv;
    for (postdiv = 256; postdiv > 1; --postdiv) {
        if (freq / (prescale * (postdiv - 1)) > baudrate)
            break;
    }

    reg(SSPCPSR) = prescale;

    uint32_t val = postdiv << 8;

    // set format to 8 bits, pol 0, pha 0, msb first
    // TODO: make it configurable
    val |= 7;

    reg(SSPCR0) = val;

    reg(SSPDMACR) = 0x3;

    reg(SSPCR1) = 1 << 1;

    init_done = true;
}

void spi::transfer(uint8_t* tx, uint8_t* rx, size_t size) {
    size_t rx_rem = size, tx_rem = size;

    while (rx_rem || tx_rem) {
        if (tx_rem && (reg(SSPSR) & (1 << 1))) {
            reg(SSPDR) = tx ? *tx++ : 0;
            tx_rem--;
        }
        if (rx_rem && (reg(SSPSR) & (1 << 2))) {
            rx_rem--;
            uint8_t val = reg(SSPDR);
            if (rx)
                *rx++ = val;
        }
    }
}

void spi::write_repeat(void* val, size_t val_size, size_t count, bool bigendian) {
    uint64_t value = 0;

    switch (val_size) {
    case 1:
        value = *static_cast<uint8_t*>(val);
        break;
    case 2:
        value = bigendian ? be16(*static_cast<uint16_t*>(val)) : *static_cast<uint16_t*>(val);
        break;
    case 4:
        value = bigendian ? be32(*static_cast<uint32_t*>(val)) : *static_cast<uint32_t*>(val);
        break;
    case 8:
        value = bigendian ? be64(*static_cast<uint64_t*>(val)) : *static_cast<uint64_t*>(val);
        break;
    default:
        throw exception("invalid value size");
    }

    auto ptr = reinterpret_cast<uint8_t*>(&value);
    while (count--) {
        for (size_t i = 0; i < val_size; ++i) {
            while (!(reg(SSPSR) & (1 << 1))) {}
            reg(SSPDR) = ptr[i];
        }
    }

    while (reg(SSPSR) & (1 << 4)) {}

    uintptr_t r = base + SSPDR;
    unsigned v;
    while (reg(SSPSR) & (1 << 2)) {
        // use asm volatile otherwise compiler will optimize it out
        asm volatile("ldr %0, [%1]" : "=r"(v) : "r"(r));
    }

    reg(SSPICR) = 1;
}

}  // namespace soc::rp2040
