/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stddef.h>
#include <stdint.h>

export module picoic.ui;

import device.gpio;
import device.i2c;
import board.peripherals;
import lib.ttf;
import std.string;
import std.memory;
import lib.fmt;
import lib.gpio;
import lib.ft6206;
import lib.ili9341;
import lib.async;
import lib.callback;
import core.event;

using lib::fmt::println;
using lib::ili9341::update_pos;
using std::string;
using std::unique_ptr;
using core::event;

namespace {

enum class alignment {
    LEFT,
    RIGHT,
    CENTER,
};

class drawable {
 public:
    virtual void draw() = 0;
};

class textbox : drawable {
 public:
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t color;
    uint16_t background;
    string text;
    size_t size;
    alignment align;

    textbox(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color, uint16_t bg,
            string text, size_t size, alignment align = alignment::LEFT)
        : x(x),
          y(y),
          width(width),
          height(height),
          color(color),
          background(bg),
          text(text),
          size(size),
          align(align) {}

    void draw() override {
        lib::ili9341::fill_rect(x, y, width, height, background);
        auto pos_x = x;

        if (align != alignment::LEFT) {
            uint16_t text_width = 0;
            lib::ili9341::text(text_width, y, size, color, background, text, update_pos::X, true);
            if (text_width > width) {
                text_width = width;
            }
            if (align == alignment::RIGHT) {
                pos_x += width - text_width;
            } else {
                pos_x += (width - text_width) / 2;
            }
        }

        lib::ili9341::text(pos_x, y, size, color, background, text);
    }
};

class button : textbox {
 public:
    bool pressed;
    uint16_t pri_color;
    uint16_t pri_bg;
    // secondary colors is when button is pressed;
    uint16_t sec_color;
    uint16_t sec_bg;
    button(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color, uint16_t bg,
           string const& text, size_t size, uint16_t sec_color, uint16_t sec_bg)
        : textbox(x, y, width, height, color, bg, text, size, alignment::CENTER),
          pressed(false),
          pri_color(color),
          pri_bg(bg),
          sec_color(sec_color),
          sec_bg(sec_bg),
          cb(nullptr) {
        lib::ft6206::register_callback(
            [this](uint8_t event, uint16_t x, uint16_t y) { event_handler(event, x, y); }, 0x3);
    }

    void draw() override {
        if (pressed) {
            color = sec_color;
            background = sec_bg;
        } else {
            color = pri_color;
            background = pri_bg;
        }
        textbox::draw();
    }

    template <typename F>
    void register_callback(F&& f) {
        cb.reset(lib::callback<bool>::create(std::forward<F>(f)));
    }

 private:
    void event_handler(uint8_t event, uint16_t ex, uint16_t ey) {
        auto prev = pressed;
        if (event == 2) {
            if (ex >= x && ex < (x + width) && ey >= y && ey < (y + height)) {
                pressed = true;
            }
        } else {
            pressed = false;
        }

        // redraw if state changed
        if (prev != pressed) {
            button::draw();
            if (cb) {
                (*cb)(pressed);
            }
        }
    }

    unique_ptr<lib::callback<bool>> cb;
};

static event done;

constexpr uint16_t BUTTON_BG_COLOR = 0x30BA;
constexpr uint16_t BUTTON_SEC_BG_COLOR = 0x0830;
constexpr uint16_t STATUS_BG_COLOR = 0x0580;

}  // namespace

export namespace picoic::ui {

void init() {

    auto& gpio = board::peripherals::default_gpio();
    gpio.set_func(4, soc::rp2040::gpio::func::SPI);
    gpio.set_func(6, soc::rp2040::gpio::func::SPI);
    gpio.set_func(7, soc::rp2040::gpio::func::SPI);

    gpio.set(5, 1);
    device::gpio::config_t cs_config = {.dir = device::gpio::dir::OUTPUT};
    gpio.config(5, cs_config);
    gpio.set(8, 1);
    gpio.config(8, cs_config);

    device::gpio::config_t i2c_config = {
        .dir = device::gpio::dir::INPUT,
        .pull = device::gpio::pull::UP,
    };
    gpio.config(2, i2c_config);
    gpio.config(3, i2c_config);

    gpio.set_func(2, soc::rp2040::gpio::func::I2C);
    gpio.set_func(3, soc::rp2040::gpio::func::I2C);

    lib::ttf::init();

    // init display
    lib::ili9341::init();
    lib::ili9341::clear_screen(0xffff);

    // init touch screen
    auto i2c = device::manager::find<device::i2c>("i2c1");
    lib::ft6206::init(i2c, 0x38, 9, 240, 320);

    textbox title(0, 0, lib::ili9341::get_width(), 34, 0xffff, STATUS_BG_COLOR, "PICO IC", 9,
            alignment::CENTER);
    title.draw();

    textbox text(0, lib::ili9341::get_height() / 2 - 15, lib::ili9341::get_width(), 30, 0, 0xffff,
            "button released", 8, alignment::CENTER);
    text.draw();

    button ok_button(10, lib::ili9341::get_height() - 40, 60, 30, 0xffff, BUTTON_BG_COLOR,
            "Ok", 8, 0xffff, BUTTON_SEC_BG_COLOR);
    ok_button.draw();
    ok_button.register_callback([&text](bool pressed) {
        if (pressed) {
            text.text = "button pressed";
        } else {
            text.text = "button released";
        }
        text.draw();
    });

    done.wait_for_signal();
}

}  // namespace picoic::ui
