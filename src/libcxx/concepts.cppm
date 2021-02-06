/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module std.concepts;

export import std.type_traits;

export namespace std {

template <typename T>
concept integral = is_integral_v<T>;

template <typename T>
concept pointer = is_pointer_v<T>;

namespace details {
template <typename T, typename U>
concept SameHelper = is_same_v<T, U>;
}

template <typename T, typename U>
concept same_as = details::SameHelper<T, U>&& details::SameHelper<U, T>;

template <typename T>
concept signed_integral = integral<T>&& is_signed_v<T>;

}  // namespace std
