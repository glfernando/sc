/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <app/shell.h>
#include <errcodes.h>
#include <stdint.h>
#include <string.h>

export module lib.spi;

import device.spi;
import lib.fmt;
import std.string;
import std.vector;
import lib.gpio;
import lib.exception;

using lib::exception;
using lib::fmt::println;
using std::string;
using std::vector;

export namespace lib::spi {

class dev {
 public:
    dev(string const& bus_name, unsigned cs_gpio = ~0U) : bus_name(bus_name), cs_gpio(cs_gpio) {}

    void init();
    void transfer(uint8_t* tx, uint8_t* rx, size_t size);
    void read(uint8_t* buf, size_t size);
    void write(uint8_t* buf, size_t size);
    void write_repeat(void* val, size_t val_size, size_t count, bool bigendian = false);
    void set_cs(bool cs);

 private:
    string const bus_name;
    device::spi* bus;
    unsigned cs_gpio;
};

void dev::init() {
    bus = device::manager::find<device::spi>(bus_name);
    if (!bus)
        throw exception("invalid bus name for spi dev");

    bus->init();
}

void dev::transfer(uint8_t* tx, uint8_t* rx, size_t size) {
    bus->transfer(tx, rx, size);
}

void dev::read(uint8_t* buf, size_t size) {
    bus->read(buf, size);
}

void dev::write(uint8_t* buf, size_t size) {
    bus->write(buf, size);
}

void dev::write_repeat(void* val, size_t val_size, size_t count, bool bigendian) {
    bus->write_repeat(val, val_size, count, bigendian);
}

void dev::set_cs(bool cs) {
    if (cs_gpio != ~0U) {
        lib::gpio::set(cs_gpio, cs);
    }
}

}  // namespace lib::spi

static void cmd_spi_usage() {
    println("spi list");
    println("spi read <name> [size]");
    println("spi write <name> <byte1> [byte2 .. byteN]");
}

static void list_spi_buses() {
    auto devices = device::manager::find_all<device::spi>();

    for (auto dev : devices)
        println("{}", dev->name());
}

static device::spi* get_device(string const& name) {
    auto devices = device::manager::find_all<device::spi>();

    for (auto dev : devices)
        if (name == dev->name())
            return dev;
    return nullptr;
}

static int cmd_spi(int argc, char const* argv[]) {
    if (argc < 2) {
        cmd_spi_usage();
        return 0;
    }

    string cmd = argv[1];

    if (cmd == "list") {
        list_spi_buses();
        return 0;
    }

    if (argc < 3) {
        cmd_spi_usage();
        return 0;
    }

    auto spi = get_device(argv[2]);
    if (spi == nullptr) {
        println("could not find device {}", argv[2]);
        return 0;
    }

    if (cmd == "read") {
        size_t size = argc == 4 ? strtoul(argv[3], NULL, 0) : 1;
        vector<uint8_t> buf;
        buf.resize(size);
        spi->transfer(nullptr, buf.data(), buf.size());
        for (auto v : buf)
            println("{:#x}", v);
    } else if (argc >= 4 && cmd == "write") {
        vector<uint8_t> buf;
        for (int i = 3; i < argc; ++i) {
            uint8_t val = strtoul(argv[i], NULL, 0);
            buf.push_back(val);
        }
        spi->transfer(buf.data(), nullptr, buf.size());
    } else {
        cmd_spi_usage();
    }

    return 0;
}

shell_declare_static_cmd(spi, "read/write from spi bus", cmd_spi, cmd_spi_usage);
