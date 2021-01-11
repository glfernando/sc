/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module lib.utils;

export namespace sc::lib::utils {

template <typename T, typename U>
constexpr auto min(T a, U b) -> decltype(a + b) {
    return b < a ? b : a;
}

template <typename T, typename U>
constexpr T align_up(T v, U a) {
    U mask = a - 1;
    return (v + mask) & ~mask;
}

}  // namespace sc::lib::utils
