/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

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

template <typename T>
constexpr T&& forward(remove_reference_t<T>&& t) noexcept {
    return static_cast<T&&>(t);
}

}  // namespace std
