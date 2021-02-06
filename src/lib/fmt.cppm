/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (C) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stdint.h>
#include <string.h>

export module lib.fmt;

export import std.string;
export import std.concepts;
import board.debug.uart;

using sc::board::debug::uart::putchar;
using sc::board::debug::uart::puts;

// module private data
namespace sc::lib::fmt {

enum fmt_flags {
    FMT_LEFT = 1 << 0,
    FMT_ZEROPAD = 1 << 1,
    FMT_CAPS = 1 << 2,
    FMT_POUND = 1 << 3,
    FMT_CHAR = 1 << 4,
};

struct modifiers {
    unsigned flags;
    unsigned width;
    unsigned base;
};

class fmt_out {
 public:
    fmt_out(std::string* s = nullptr) : str_p(s) {}

    fmt_out& operator<<(char c) {
        if (str_p)
            *str_p += c;
        else
            putchar(c);
        return *this;
    }
    fmt_out& operator<<(const char* c_str) {
        if (str_p)
            *str_p += c_str;
        else
            puts(c_str);
        return *this;
    }

 private:
    std::string* str_p;
};

//
// Parse format string modifiers. e.g {:<08x}
//
modifiers parse_modifiers(const std::string& flags) {
    modifiers m{0, 0, 10};

    if (flags.size() == 0)
        return m;

    unsigned& f = m.flags;
    unsigned& w = m.width;
    unsigned& b = m.base;

    int index = 0;
    char c = flags[index++];

    if (c == ':')
        c = flags[index++];

    if (!c)
        return m;

    if (c == '<' || c == '>') {
        f |= c == '<' ? FMT_LEFT : 0;
        if (!(c = flags[index++]))
            return m;
    }

    if (c == '#') {
        f |= FMT_POUND;
        c = flags[index++];
    }

    if (c == '0') {
        f |= FMT_ZEROPAD;
        c = flags[index++];
    }

    while (c) {
        if (c < '0' || c > '9')
            break;
        w = w * 10 + c - '0';
        c = flags[index++];
    }

    if (!c)
        return m;

    // check for base modifier
    switch (c) {
    case 'd':
        break;
    case 'X':
        f |= FMT_CAPS;
    case 'x':
        b = 16;
        break;
    case 'o':
        b = 8;
        break;
    case 'c':
        // force char conversion (only valid for integrals)
        f |= FMT_CHAR;
        break;
    }
    return m;
}

//
// Print c-style string to fmt_out
//
void fmt_print_string(const char* s, int w, int f, fmt_out fout = {}) {
    char pad = f & FMT_ZEROPAD ? '0' : ' ';
    bool hex_prefix = f & FMT_POUND;

    int len = strlen(s);

    if (hex_prefix)
        len += 2;

    // if zero pad add prefix before padding
    if (hex_prefix && (f & FMT_ZEROPAD)) {
        fout << "0x";
        len -= 2;
    }

    if (!(f & FMT_LEFT))
        while (len < w--)
            fout << pad;

    if (hex_prefix && !(f & FMT_ZEROPAD)) {
        fout << "0x";
        len -= 2;
    }
    fout << s;

    while (len < w--)
        fout << pad;
}

void fmt_print_integer(uint64_t n, bool neg, modifiers& mod, fmt_out fout = {}) {
    auto& [f, w, b] = mod;

    // check if we are printing an char
    if (f & FMT_CHAR) {
        char str[] = {static_cast<char>(n), '\0'};
        return fmt_print_string(str, w, f, fout);
    }

    if (n == 0)
        return fmt_print_string("0", w, f, fout);

    // 22 chars fit 64 bits integer
    constexpr unsigned NUM_BUF_MAX = 22;
    char num_buf[NUM_BUF_MAX];
    char* s = num_buf + NUM_BUF_MAX - 1;
    *s = '\0';

    // mask and shift for base 8 and 16 (to avoid division)
    unsigned mask = b - 1;
    unsigned shift = 3 + (b >> 4);

    int t;
    int l = f & FMT_CAPS ? 'A' : 'a';
    while (n) {
        if (b == 10) {
            t = n % b;
            n /= b;
        } else {
            t = n & mask;
            n >>= shift;
        }

        if (t > 9)
            t += l - '0' - 10;
        *--s = t + '0';
    }

    if (neg) {
        if (w && (f & FMT_ZEROPAD)) {
            fout << '-';
            --w;
        } else {
            *--s = '-';
        }
    }

    // now num is a c-style string we can use fmt_print_string for printing
    fmt_print_string(s, w, f, fout);
}

}  // namespace sc::lib::fmt

// public (exported) part
export namespace sc::lib::fmt {

//
// Ends print template recursion. This handles the case when only fmt is specified
//
void print(const char* fmt) {
    puts(fmt);
}

//
// Format printing to default console output
//
template <typename T, typename... Args>
void print(const char* fmt, T&& val, Args&&... args) {
    // remove reference for doing type comparations
    using U = std::remove_reference_t<T>;
    while (*fmt) {
        if (*fmt == '{' && *++fmt != '{') {
            std::string flags;
            for (; *fmt && *fmt != '}'; ++fmt)
                flags += *fmt;
            if (!*fmt++)
                return;  // "missing '}' in format"

            auto mod = parse_modifiers(flags);

            if constexpr (std::integral<U>) {
                bool neg = mod.base == 10 && std::signed_integral<U> && val < 0;
                auto num = neg ? -static_cast<int64_t>(val) : val;
                fmt_print_integer(num, neg, mod);
            } else if constexpr (std::same_as<U, const char*> || std::same_as<U, char*>) {
                fmt_print_string(val ?: "(nullptr)", mod.width, mod.flags);
            } else if constexpr (std::pointer<U>) {
                auto p = reinterpret_cast<uintptr_t>(val);
                mod.base = 16;
                mod.flags |= FMT_POUND;
                fmt_print_integer(p, false, mod);
            } else if constexpr (std::same_as<U, std::string>) {
                fmt_print_string(val.c_str(), mod.width, mod.flags);
            } else {
                static_assert(std::is_convertible_v<U, std::string>, "fmt: unsupported type");
                fmt_print_string(std::string(val).c_str(), mod.width, mod.flags);
            }
            print(fmt, std::forward<Args>(args)...);
            return;
        }
        if (*fmt == '}' && *++fmt != '}')
            return;  // "unmatched '}' in format"
                     // hh::cout << *fmt++;
        putchar(*fmt++);
    }
}

//
// Ends sprint template recursion. This handles the case when only fmt is specified
//
void sprint(std::string& str, const char* fmt) {
    str += fmt;
}

//
// Format printing to std::string
//
template <typename T, typename... Args>
void sprint(std::string& str, const char* fmt, T&& val, Args&&... args) {
    // remove reference for doing type comparations
    using U = std::remove_reference_t<T>;
    while (*fmt) {
        if (*fmt == '{' && *++fmt != '{') {
            std::string flags;
            for (; *fmt && *fmt != '}'; ++fmt)
                flags += *fmt;
            if (!*fmt++)
                return;  // missing '}' in format

            auto mod = parse_modifiers(flags);

            if constexpr (std::integral<U>) {
                bool neg = mod.base == 10 && std::signed_integral<T> && val < 0;
                if (neg)
                    val = -static_cast<int64_t>(val);

                fmt_print_integer(val, neg, mod, &str);
            } else if constexpr (std::same_as<U, const char*> || std::same_as<U, char*>) {
                fmt_print_string(val ?: "(nullptr)", mod.width, mod.flags, &str);
            } else if constexpr (std::pointer<U>) {
                auto p = reinterpret_cast<uintptr_t>(val);
                mod.base = 16;
                mod.flags |= FMT_POUND;
                fmt_print_integer(p, false, mod, &str);
            } else if constexpr (std::same_as<U, std::string>) {
                fmt_print_string(val.c_str(), mod.width, mod.flags, &str);
            } else {
                static_assert(std::is_convertible_v<U, std::string>, "fmt: unsupported type");
                fmt_print_string(std::string(val).c_str(), mod.width, mod.flags, &str);
            }
            return sprint(str, fmt, std::forward<Args>(args)...);
        }
        if (*fmt == '}' && *++fmt != '}')
            return;  // unmatched '}' in format
        str += *fmt++;
    }
}

template <typename... Args>
std::string sprint(const char* fmt, Args&&... args) {
    std::string str;
    sprint(str, fmt, std::forward<Args>(args)...);
    return str;
}

template <typename... Args>
void println(const char* fmt, Args&&... args) {
    print(fmt, std::forward<Args>(args)...);
    putchar('\n');
}

}  // namespace sc::lib::fmt
