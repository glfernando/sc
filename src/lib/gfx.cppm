/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <endian.h>
#include <stdint.h>

export module lib.gfx;

export import std.memory;

using std::unique_ptr;

namespace {

namespace rgb565 {

uint8_t blend_alpha_color(uint16_t fb_color, uint16_t bg_color, uint8_t alpha) {
    if (alpha == 255) {
        return fb_color;
    }
    if (alpha == 0) {
        return bg_color;
    }

    uint16_t color = fb_color * alpha + bg_color * (255 - alpha);
    return color / 255;
}

union color {
    struct {
        uint16_t blue : 5;
        uint16_t green : 6;
        uint16_t red : 5;
    };
    uint16_t data;
};
static_assert(sizeof(color) == 2);

}  // namespace rgb565

}  // namespace

export namespace lib::gfx {

unique_ptr<uint8_t[]> bitmap_to_565(uint8_t* buf, unsigned w, unsigned h, uint16_t fg_color,
                                    uint16_t bg_color, bool bigendian = false) {
    unique_ptr<uint8_t[]> out{new uint8_t[w * h * 2]};
    auto ptr = reinterpret_cast<uint16_t*>(out.get());

    rgb565::color fg = {.data = fg_color};
    rgb565::color bg = {.data = bg_color};

    for (unsigned i = 0; i < h; ++i) {
        for (unsigned j = 0; j < w; ++j) {
            uint8_t alpha = buf[i * w + j];
            rgb565::color color;
            color.blue = rgb565::blend_alpha_color(fg.blue, bg.blue, alpha);
            color.green = rgb565::blend_alpha_color(fg.green, bg.green, alpha);
            color.red = rgb565::blend_alpha_color(fg.red, bg.red, alpha);

            ptr[i * w + j] = bigendian ? be16(color.data) : color.data;
        }
    }

    return out;
}

}  // namespace lib::gfx
