/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stddef.h>

export module std.type_traits;

export namespace std {

// NOTE: use compiler builtins when possible

template <bool val>
struct bool_constant {
    using type = bool_constant<val>;
    static constexpr bool value = val;
};

using true_type = bool_constant<true>;
using false_type = bool_constant<false>;

template <bool, typename T = void>
struct enable_if {};

template <typename T>
struct enable_if<true, T> {
    using type = T;
};

template <bool cond, typename T = void>
using enable_if_t = typename enable_if<cond, T>::type;

// is integral
template <typename T>
struct is_integral {
    static const bool value = __is_integral(T);
};

template <class T>
inline constexpr bool is_integral_v = is_integral<T>::value;

// is signed
template <typename T>
struct is_signed {
    static const bool value = __is_signed(T);
};

template <class T>
inline constexpr bool is_signed_v = is_signed<T>::value;

// is_union
template <typename T>
struct is_union {
    static const bool value = __is_union(T);
};

template <class T>
inline constexpr bool is_union_v = is_union<T>::value;

// is_pointer
template <typename T>
struct is_pointer {
    static const bool value = __is_pointer(T);
};

template <class T>
inline constexpr bool is_pointer_v = is_pointer<T>::value;

// is same
template <typename T, typename U>
struct is_same {
    static const bool value = __is_same(T, U);
};

template <typename T, typename U>
inline constexpr bool is_same_v = is_same<T, U>::value;

template <typename From, typename To>
struct is_convertible {
    static const bool value = __is_convertible(From, To);
};

template <typename From, typename To>
inline constexpr bool is_convertible_v = is_convertible<From, To>::value;

// remove cv
// clang-format off
template<typename T> struct remove_cv                   { typedef T type; };
template<typename T> struct remove_cv<const T>          { typedef T type; };
template<typename T> struct remove_cv<volatile T>       { typedef T type; };
template<typename T> struct remove_cv<const volatile T> { typedef T type; };

template<typename T> struct remove_const                { typedef T type; };
template<typename T> struct remove_const<const T>       { typedef T type; };

template<typename T> struct remove_volatile             { typedef T type; };
template<typename T> struct remove_volatile<volatile T> { typedef T type; };
// clang-format on

template <typename T>
using remove_cv_t = typename remove_cv<T>::type;
template <typename T>
using remove_const_t = typename remove_const<T>::type;
template <typename T>
using remove_volatile_t = typename remove_volatile<T>::type;

// remove reference
// clang-format off
template<typename T> struct remove_reference      {typedef T type;};
template<typename T> struct remove_reference<T&>  {typedef T type;};
template<typename T> struct remove_reference<T&&> {typedef T type;};
// clang-format on

template <typename T>
using remove_reference_t = typename remove_reference<T>::type;

// forward
template <typename T>
constexpr T&& forward(remove_reference_t<T>& t) noexcept {
    return static_cast<T&&>(t);
}

// move
template <typename T>
constexpr remove_reference_t<T>&& move(T&& t) noexcept {
    return static_cast<remove_reference_t<T>&&>(t);
}

template <typename T>
constexpr T&& forward(remove_reference_t<T>&& t) noexcept {
    return static_cast<T&&>(t);
}

// remove extent
template <typename T>
struct remove_extent {
    using type = T;
};

template <typename T>
struct remove_extent<T[]> {
    using type = T;
};

template <typename T, size_t N>
struct remove_extent<T[N]> {
    using type = T;
};

template <class T>
using remove_extent_t = typename remove_extent<T>::type;

// conditional
template <bool B, typename T, typename F>
struct conditional {
    using type = T;
};

template <typename T, typename F>
struct conditional<false, T, F> {
    using type = F;
};

template <bool B, typename T, typename F>
using conditional_t = typename conditional<B, T, F>::type;

// is array
template <typename T>
struct is_array {
    static const bool value = __is_array(T);
};

template <class T>
inline constexpr bool is_array_v = is_array<T>::value;

// is function
template <typename T>
struct is_function {
    static const bool value = __is_function(T);
};

template <class T>
inline constexpr bool is_function_v = is_function<T>::value;

// add pointer
namespace detail {
template <typename T>
struct type_identity {
    using type = T;
};  // or use std::type_identity (since C++20)

template <typename T>
auto try_add_pointer(int) -> type_identity<typename std::remove_reference<T>::type*>;
template <typename T>
auto try_add_pointer(...) -> type_identity<T>;

}  // namespace detail

template <typename T>
struct add_pointer : decltype(detail::try_add_pointer<T>(0)) {};

template <class T>
using add_pointer_t = typename add_pointer<T>::type;

// decay
template <typename T>
struct decay {
 private:
    using U = remove_reference_t<T>;

 public:
    typedef std::conditional_t<
        std::is_array_v<U>, std::remove_extent_t<U>*,
        std::conditional_t<std::is_function_v<U>, std::add_pointer_t<U>, std::remove_cv_t<U>>>
        type;
};

template <typename T>
using decay_t = typename decay<T>::type;

// is_trivial
template <typename T>
struct is_trivial {
    static const bool value = __is_trivial(T);
};

template <class T>
inline constexpr bool is_trivial_v = is_trivial<T>::value;

// __is_standard_layout
template <typename T>
struct is_standard_layout {
    static const bool value = __is_standard_layout(T);
};

template <class T>
inline constexpr bool is_standard_layout_v = is_standard_layout<T>::value;

// is_trivially_constructible
template <typename T>
struct is_trivially_constructible {
    static const bool value = __is_trivially_constructible(T);
};

template <class T>
inline constexpr bool is_trivially_constructible_v = is_trivially_constructible<T>::value;

template <typename T, T v>
struct integral_constant {
    static constexpr T value = v;
    using value_type = T;
    using type = integral_constant;
    constexpr operator value_type() const noexcept { return value; }
    constexpr value_type operator()() const noexcept { return value; }
};

}  // namespace std
