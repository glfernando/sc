/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <app/shell.h>
#include <endian.h>
#include <errcodes.h>
#include <stdint.h>
#include <string.h>

export module lib.ili9341;

import device.spi;
import lib.fmt;
import std.string;
import std.vector;
import lib.time;
import lib.timestamp;
import lib.gpio;
import std.tuple;
import lib.ttf;
import lib.gfx;
import lib.spi;

using lib::fmt::println;
using std::string;
using std::tuple;
using std::vector;
using namespace lib::time;

constexpr unsigned DC_GPIO = 8;

static bool bus_init;
static lib::spi::dev ili9341_spi("spi0", 5);

static lib::spi::dev& get_device() {
    if (!bus_init) {
        bus_init = true;
        ili9341_spi.init();
    }
    return ili9341_spi;
}

static void write_cmd(uint8_t cmd) {
    auto& spi = get_device();

    lib::gpio::set(DC_GPIO, false);
    delay(1us);
    spi.transfer(&cmd, nullptr, 1);
    lib::gpio::set(DC_GPIO, true);
    delay(1us);
}

static void read(uint8_t addr, uint8_t* buffer, size_t size) {
    auto& spi = get_device();

    spi.set_cs(false);
    delay(1us);

    write_cmd(addr);

    spi.transfer(nullptr, buffer, size);
    delay(1us);
    spi.set_cs(true);
}

static void write(uint8_t addr, uint8_t* buffer = nullptr, size_t size = 0) {
    auto& spi = get_device();

    spi.set_cs(false);
    delay(1us);

    write_cmd(addr);

    if (size)
        spi.transfer(buffer, nullptr, size);
    delay(1us);
    spi.set_cs(true);
}

void spi_write16(uint16_t v) {
    auto& spi = get_device();

    uint8_t buf[2];
    buf[0] = v >> 8;
    buf[1] = v;

    spi.transfer(buf, nullptr, sizeof buf);
}

void set_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    uint16_t x2 = x + w - 1;
    uint16_t y2 = y + h - 1;

    write_cmd(0x2a);
    spi_write16(x);
    spi_write16(x2);

    write_cmd(0x2b);
    spi_write16(y);
    spi_write16(y2);

    write_cmd(0x2c);
}

static uint16_t height = 320;
static uint16_t width = 240;

export namespace lib::ili9341 {

uint16_t get_width() {
    return width;
}

uint16_t get_height() {
    return height;
}

void clear_screen(uint16_t color) {
    auto& spi = get_device();

    spi.set_cs(false);
    delay(1us);

    set_window(0, 0, width, height);
    spi.write_repeat(&color, sizeof color, height * width, true);

    delay(1us);
    spi.set_cs(true);
}

void pixel(uint16_t x, uint16_t y, uint16_t color) {
    auto& spi = get_device();

    spi.set_cs(false);
    delay(1us);

    set_window(x, y, 1, 1);
    spi_write16(color);

    delay(1us);
    spi.set_cs(true);
}

void write_char(auto bitmap, uint16_t x, uint16_t y, uint16_t color, uint16_t bg) {
    uint16_t height = bitmap.height;
    uint16_t width = bitmap.width;

    auto buf = lib::gfx::bitmap_to_565((uint8_t*)bitmap.buffer, width, height, color, bg, true);

    auto& spi = get_device();

    spi.set_cs(false);
    delay(1us);

    set_window(x, y, width, height);
    spi.transfer(buf.get(), nullptr, width * height * 2);

    delay(1us);
    spi.set_cs(true);
}

void fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    auto& spi = get_device();

    spi.set_cs(false);
    delay(1us);

    set_window(x, y, w, h);
    spi.write_repeat(&color, sizeof color, h * w, true);

    delay(1us);
    spi.set_cs(true);
}

void vline(uint16_t x, uint16_t y, uint16_t len, uint16_t thickness, uint16_t color) {
    fill_rect(x, y, thickness, len, color);
}

void hline(uint16_t x, uint16_t y, uint16_t len, uint16_t thickness, uint16_t color) {
    fill_rect(x, y, len, thickness, color);
}

void rect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color) {
    uint16_t x1 = x0 + w;
    uint16_t y1 = y0 + h;

    hline(x0, y0, w, 1, color);
    hline(x0, y1, w, 1, color);

    vline(x0, y0, h, 1, color);
    vline(x1, y0, h, 1, color);
}

void scroll_area(uint16_t top, uint16_t bottom) {
    uint16_t area = 320 - top - bottom;

    uint8_t data[6];
    data[0] = top >> 8;
    data[1] = top & 0xff;
    data[2] = area >> 8;
    data[3] = area & 0xff;
    data[4] = bottom >> 8;
    data[5] = bottom & 0xff;

    write(0x33, data, sizeof data);
}

void scroll(uint16_t lines, uint16_t top, uint16_t bottom) {
    println("scroll lines {}", lines);

    scroll_area(top, bottom);

    uint8_t data[2];
    data[0] = lines >> 8;
    data[1] = lines & 0xff;
    write(0x37, data, sizeof data);
}

struct text_size {
    uint16_t width;
    uint16_t height;
};

enum class update_pos : unsigned {
    NONE,
    X,
    Y,
    BOTH,
};

void text(uint16_t& x, uint16_t& y, unsigned size, uint16_t color, uint16_t bg, string const& str,
        update_pos update, bool dryrun = false) {
    auto pos_x = x;
    for (auto c : str) {
        auto bitmap = lib::ttf::char_to_bitmap(c, size);
        //println("bitmapp width {}, height {}, buf {}", bitmap.width, bitmap.height, bitmap.buffer);
        //println("left {}, top {}", bitmap.left, bitmap.top);
        if (!dryrun) {
            lib::ili9341::write_char(bitmap, pos_x + bitmap.left, y + bitmap.top, color, bg);
        }
        pos_x += bitmap.advance;
    }

    if (update == update_pos::Y || update == update_pos::BOTH) {
        y += ttf::char_height(size);
    }
    if (update == update_pos::X || update == update_pos::BOTH) {
        x += pos_x;
    }
}

void text(uint16_t x, uint16_t y, unsigned size, uint16_t color, uint16_t bg, string const& str) {
    text(x, y, size, color, bg, str, update_pos::NONE);
}

/*
template<class T = uint16_t>
void text(T&& x, T&& y, unsigned size, uint16_t color, uint16_t bg, string const& str,
        update_pos update = update_pos::NONE) {
    text(x, y, size, color, bg, str, update);
}*/

void init() {
    write(0x1);
    delay(150ms);

    uint8_t cmd1[] = {0x03, 0x80, 0x02};
    write(0xef, cmd1, sizeof cmd1);

    uint8_t cmd2[] = {0x00, 0xc1, 0x30};
    write(0xcf, cmd2, sizeof cmd2);

    uint8_t cmd3[] = {0x64, 0x03, 0x12, 0x81};
    write(0xed, cmd3, sizeof cmd3);

    uint8_t cmd4[] = {0x85, 0x00, 0x78};
    write(0xe8, cmd4, sizeof cmd4);

    uint8_t cmd5[] = {0x39, 0x2C, 0x00, 0x34, 0x02};
    write(0xcb, cmd5, sizeof cmd5);

    uint8_t cmd6[] = {0x20};
    write(0xf7, cmd6, sizeof cmd6);

    uint8_t cmd7[] = {0x00, 0x00};
    write(0xea, cmd7, sizeof cmd7);

    uint8_t cmd8[] = {0x23};
    write(0xc0, cmd8, sizeof cmd8);

    uint8_t cmd9[] = {0x10};
    write(0xc1, cmd9, sizeof cmd9);

    uint8_t cmd10[] = {0x3e, 0x28};
    write(0xc5, cmd10, sizeof cmd10);

    uint8_t cmd11[] = {0x86};
    write(0xc7, cmd11, sizeof cmd11);

    uint8_t cmd12[] = {0x48};
    write(0x36, cmd12, sizeof cmd12);

    uint8_t cmd13[] = {0x00};
    write(0x37, cmd13, sizeof cmd13);

    uint8_t cmd14[] = {0x55};
    write(0x3a, cmd14, sizeof cmd14);

    uint8_t cmd15[] = {0x00, 0x18};
    write(0xb1, cmd15, sizeof cmd15);

    uint8_t cmd16[] = {0x08, 0x82, 0x27};
    write(0xb6, cmd16, sizeof cmd16);

    uint8_t cmd17[] = {0x0};
    write(0xf2, cmd17, sizeof cmd17);

    uint8_t cmd18[] = {0x1};
    write(0x26, cmd18, sizeof cmd18);

    uint8_t cmd19[] = {0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
                       0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00};
    write(0xe0, cmd19, sizeof cmd19);

    uint8_t cmd20[] = {0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
                       0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F};
    write(0xe1, cmd20, sizeof cmd20);

    write(0x11);
    delay(150ms);

    write(0x29);
    delay(150ms);
}

}  // namespace lib::ili9341

void cmd_ili9341_usage() {
    println("ili9341 read <cmd> [size]");
    println("ili9341 write <cmd> <byte1> [byte2 .. byteN]");
    println("ili9341 pixel x y color");
    println("ili9341 text size color bg_color string");
    println("ili9341 fill-rect x y width height color");
    println("ili9341 rect x y width height color");
    println("ili9341 scroll lines");
    println("ili9341 scroll-test [top] [bottom]");
}

static int cmd_ili9341(int argc, char const* argv[]) {
    if (argc < 2) {
        cmd_ili9341_usage();
        return 0;
    }

    string cmd = argv[1];
    if (cmd == "init") {
        lib::ili9341::init();
        return 0;
    } else if (cmd == "clear") {
        uint16_t color = argc == 3 ? strtoul(argv[2], NULL, 0) : 0;
        auto t0 = lib::timestamp::us();
        lib::ili9341::clear_screen(color);
        auto t1 = lib::timestamp::us();
        println("clear screen took {}us", t1 - t0);
        return 0;
    }

    if (argc >= 3 && cmd == "read") {
        uint8_t addr = strtoul(argv[2], NULL, 0);
        size_t size = argc == 4 ? strtoul(argv[3], NULL, 0) : 1;
        vector<uint8_t> buf;
        buf.resize(size);
        read(addr, buf.data(), buf.size());
        for (auto v : buf)
            println("{:#x}", v);
    } else if (argc >= 3 && cmd == "write") {
        uint8_t addr = strtoul(argv[2], NULL, 0);
        vector<uint8_t> buf;
        for (int i = 3; i < argc; ++i) {
            uint8_t val = strtoul(argv[i], NULL, 0);
            buf.push_back(val);
        }
        write(addr, buf.data(), buf.size());
    } else if (argc == 5 && cmd == "pixel") {
        uint16_t x = strtoul(argv[2], NULL, 0);
        uint16_t y = strtoul(argv[3], NULL, 0);
        uint16_t c = strtoul(argv[4], NULL, 0);
        lib::ili9341::pixel(x, y, c);
    } else if (cmd == "text") {
        unsigned size = 16;
        uint16_t color = 0;
        uint16_t bg = 0;
        string str = "test";
        if (argc >= 3)
            size = strtoul(argv[2], NULL, 0);
        if (argc >= 4)
            color = strtoul(argv[3], NULL, 0);
        if (argc >= 5)
            bg = strtoul(argv[4], NULL, 0);
        if (argc >= 6) {
            str = argv[5];
            for (int i = 6; i < argc; ++i) {
                str += ' ';
                str += argv[i];
            }
        }
        uint16_t x = 0, y = 0;
        lib::ili9341::text(x, y, size, color, bg, str);
    } else if (argc == 7 && cmd == "fill-rect") {
        uint16_t x = strtoul(argv[2], NULL, 0);
        uint16_t y = strtoul(argv[3], NULL, 0);
        uint16_t w = strtoul(argv[4], NULL, 0);
        uint16_t h = strtoul(argv[5], NULL, 0);
        uint16_t c = strtoul(argv[6], NULL, 0);
        lib::ili9341::fill_rect(x, y, w, h, c);
    } else if (argc == 7 && cmd == "rect") {
        uint16_t x = strtoul(argv[2], NULL, 0);
        uint16_t y = strtoul(argv[3], NULL, 0);
        uint16_t w = strtoul(argv[4], NULL, 0);
        uint16_t h = strtoul(argv[5], NULL, 0);
        uint16_t c = strtoul(argv[6], NULL, 0);
        lib::ili9341::rect(x, y, w, h, c);
    } else if (argc == 3 && cmd == "scroll") {
        uint16_t lines = strtoul(argv[2], NULL, 0);
        lib::ili9341::scroll(lines, 0, 0);
    } else if (cmd == "scroll-test") {
        uint16_t top = 0;
        uint16_t bottom = 0;
        if (argc >= 3)
            top = strtoul(argv[2], NULL, 0);
        if (argc >= 4)
            bottom = strtoul(argv[3], NULL, 0);
        for (uint16_t i = top; i < 320 - bottom; ++i) {
            lib::ili9341::scroll(i, top, bottom);
            delay(20ms);
        }
    } else {
        cmd_ili9341_usage();
    }

    return 0;
}

shell_declare_static_cmd(ili9341, "ili9341 commands", cmd_ili9341, cmd_ili9341_usage);
