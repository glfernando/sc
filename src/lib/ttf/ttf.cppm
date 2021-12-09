/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <ft2build.h>
#include <stddef.h>
#include <stdint.h>
#include FT_GLYPH_H
#include FT_FREETYPE_H

extern "C" const uint8_t _font_roboto_start[];
extern "C" const uint8_t _font_roboto_end[];
extern "C" const uint8_t _font_lato_start[];
extern "C" const uint8_t _font_lato_end[];

export module lib.ttf;

import lib.fmt;
import std.string;
import lib.exception;

using lib::exception;
using lib::fmt::println;
using lib::fmt::sprint;
using std::string;

static FT_Library library;
static bool library_init;
static FT_Face font_face;

namespace {

struct font_buf {
    FT_Byte const* ptr;
    size_t size;
};

font_buf get_by_name(string const& name) {
    const void* ptr;
    size_t size;
    if (name == "roboto") {
        size = static_cast<size_t>(_font_roboto_end - _font_roboto_start);
        ptr = _font_roboto_start;
    } else if (name == "lato") {
        size = static_cast<size_t>(_font_lato_end - _font_lato_start);
        ptr = _font_lato_start;
    } else {
        throw exception("font not found");
    }

    return {static_cast<FT_Byte const*>(ptr), size};
}

constexpr unsigned DEFAULT_DPI = 148;

static unsigned dpi;

}  // namespace

export namespace lib::ttf {

void init(string const& name = "roboto", unsigned dpi_ = DEFAULT_DPI) {
    if (library_init) {
        return;
    }

    dpi = dpi_;

    FT_Error err = FT_Init_FreeType(&library);
    if (err) {
        throw exception(sprint("failed to init freetype library {}", err));
    }

    auto [font_buffer, font_size] = get_by_name(name);
    if (!font_buffer) {
        throw exception("font not found");
    }

    err = FT_New_Memory_Face(library, font_buffer, font_size, 0, &font_face);
    if (err) {
        throw exception(sprint("failed to create new memory face {}", err));
    }

    library_init = true;
}

struct font_bitmap {
    unsigned width;
    unsigned height;
    unsigned advance;
    int left;
    int top;
    void* buffer;
};

font_bitmap char_to_bitmap(unsigned c, unsigned size) {
    FT_Error err = FT_Set_Char_Size(font_face, 0, size * 64, dpi, 0);
    if (err) {
        throw exception("failed to set char size");
    }

    auto metrics_height = font_face->size->metrics.height >> 6;
    //println("metrics height {}", metrics_height);
    FT_GlyphSlot slot = font_face->glyph;
    err = FT_Load_Char(font_face, c, FT_LOAD_RENDER);
    if (err) {
        throw exception("failed to load char");
    }

    FT_Bitmap* bitmap = &slot->bitmap;

    font_bitmap bm;
    bm.width = bitmap->width;
    bm.height = bitmap->rows;
    bm.left = slot->bitmap_left;
    bm.top = metrics_height - slot->bitmap_top;
    bm.advance = slot->advance.x >> 6;
    bm.buffer = bitmap->buffer;

    return bm;
}

uint16_t char_height(unsigned size) {
    FT_Error err = FT_Set_Char_Size(font_face, 0, size * 64, dpi, 0);
    if (err) {
        throw exception("failed to set char size");
    }

    return font_face->size->metrics.height >> 6;
}

}  // namespace lib::ttf
