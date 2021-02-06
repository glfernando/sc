/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>

export module lib.hexdump;

import lib.fmt;
import lib.utils;

using namespace sc::lib::utils;
using sc::lib::fmt::print;
using sc::lib::fmt::println;

namespace {

void ascii_dump(void const* data, size_t size) {
    auto ptr = static_cast<char const*>(data);
    for (size_t i = 0; i < size; i++) {
        char c = ptr[i];
        print("{:c}", isprint(c) ? c : '.');
    }
}

}  // namespace

export namespace sc::lib {

void hexdump(void const* data, size_t size, size_t group = 4, bool offset = false,
             bool ascii = false) {
    if (data == nullptr || size == 0)
        return;

    if (group == 0)
        group = 4;  // default to 4

    // make sure group is power of 2 and not bigger than 8
    if (((group - 1) & group) || group > 8) {
        println("hexdump: invlid group {}", group);
        return;
    }

    while (size) {
        auto len = min(size, 16U);
        auto ptr = static_cast<uint8_t const*>(data);

        if (offset)
            print("{:x}: ", reinterpret_cast<uintptr_t>(ptr));

        size_t rem;
        for (rem = len; rem >= group; rem -= group, ptr += group) {
            if (len - rem)
                print(" ");
            switch (group) {
            case 1:
                print("{:02x}", *ptr);
                break;
            case 2:
                print("{:04x}", *reinterpret_cast<uint16_t const*>(ptr));
                break;
            case 4:
                print("{:08x}", *reinterpret_cast<uint32_t const*>(ptr));
                break;
            case 8:
                print("{:016x}", *reinterpret_cast<uint64_t const*>(ptr));
                break;
            }
        }

        // print remaining data
        if (rem) {
            if (len > group)
                print(" ");
            // align to the left
            for (size_t i = group - rem; i; --i)
                print("  ");
            while (rem--)
                print("{:02x}", *ptr++);
        }

        if (ascii) {
            if (len != 16) {
                // fill with spaces up to the right alignment
                size_t spaces = 16 - align_up(len, group);
                spaces = spaces * 2 + spaces / group;
                while (spaces--)
                    print(" ");
            }
            print("  ");
            ascii_dump(data, len);
        }
        println("");

        data = static_cast<uint8_t const*>(data) + len;
        size -= len;
    }
}

}  // namespace sc::lib
