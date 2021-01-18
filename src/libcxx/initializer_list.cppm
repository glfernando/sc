/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module std.initializer_list;

#include <stddef.h>

export namespace std {

template <typename T>
class initializer_list {
 public:
    inline constexpr initializer_list() noexcept : b(nullptr), s(0) {}
    inline size_t size() const noexcept { return s; }
    inline constexpr const T* begin() const noexcept { return b; }
    inline constexpr const T* end() const noexcept { return b + s; }

 private:
    const T* b;
    size_t s;
    inline constexpr initializer_list(const T* b, size_t s) noexcept : b(b), s(s) {}
};

template <typename T>
inline constexpr const T* begin(initializer_list<T> il) noexcept {
    return il.begin();
}

template <typename T>
inline constexpr const T* end(initializer_list<T> il) noexcept {
    return il.end();
}

}  // namespace std
