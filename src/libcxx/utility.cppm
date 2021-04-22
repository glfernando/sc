/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stddef.h>

export module std.utility;

export import std.type_traits;

export namespace std {

template <typename T, T... Ints>
struct integer_sequence {
    using value_type = T;
    static constexpr size_t size() noexcept { return sizeof...(Ints); }
};

template <size_t... I>
using index_sequence = integer_sequence<size_t, I...>;

template <typename T, T E>
using __make_integer_sequence = __make_integer_seq<integer_sequence, T, E>;

template <typename T, T N>
using make_integer_sequence = __make_integer_sequence<T, N>;

template <size_t N>
using make_index_sequence = make_integer_sequence<size_t, N>;

template <typename... Ts>
using index_sequence_for = make_index_sequence<sizeof...(Ts)>;

}  // namespace std
