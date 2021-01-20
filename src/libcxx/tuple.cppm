/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stddef.h>
#include <stdint.h>

export module std.tuple;

export import std.type_traits;

export namespace std {

namespace details {

template <size_t index, typename T>
class tuple_impl {
 public:
    tuple_impl() : val(T{}) {}
    tuple_impl(T const& v) : val(v) {}
    tuple_impl(T&& v) : val(std::move(v)) {}

    T& get() { return val; }
    T const& get() const { return val; }

 private:
    T val;
};

template <size_t index, typename...>
class tuple_base {};

template <size_t Index, typename Head, typename... Tail>
class tuple_base<Index, Head, Tail...> : public tuple_impl<Index, Head>,
                                         public tuple_base<Index + 1, Tail...> {
 public:
    tuple_base() = default;
    template <typename E, typename... Es>
    tuple_base(E&& elem, Es&&... elems)
        : tuple_impl<Index, E>(std::forward<E>(elem)),
          tuple_base<Index + 1, Tail...>(std::forward<Es>(elems)...) {}
};

}  // namespace details

template <typename... Ts>
class tuple : public details::tuple_base<0, Ts...> {
 public:
    template <typename... Es>
    tuple(Es&&... elems) : details::tuple_base<0, Ts...>(std::forward<Es>(elems)...) {}
};

template <typename E>
struct tuple_size;

template <typename... Es>
struct tuple_size<tuple<Es...>> : public integral_constant<size_t, sizeof...(Es)> {};

template <typename T>
inline constexpr size_t tuple_size_v = tuple_size<T>::value;

template <size_t Index, typename... Ts>
struct tuple_element {
    // valid cases will always prefer the tuple_element specialization, if this one is used it means
    // the index is invalid
    static_assert(Index < tuple_size<tuple<Ts...>>::value, "bad tuple index");
};

template <size_t Index, typename Head, typename... Tail>
struct tuple_element<Index, Head, Tail...> {
    using type = typename tuple_element<Index - 1, Tail...>::type;
};

template <typename Head, typename... Tail>
struct tuple_element<0, Head, Tail...> {
    using type = Head;
};

template <size_t Index, typename... Ts>
using tuple_element_t = typename tuple_element<Index, Ts...>::type;

template <size_t Index, typename... Ts>
constexpr auto& get(tuple<Ts...>& ts) noexcept {
    return (static_cast<details::tuple_impl<Index, tuple_element_t<Index, Ts...>>&>(ts)).get();
}

template <size_t Index, typename... Ts>
constexpr auto&& get(tuple<Ts...>&& ts) noexcept {
    return (static_cast<details::tuple_impl<Index, tuple_element_t<Index, Ts...>>&&>(ts)).get();
}

template <size_t Index, typename... Ts>
constexpr const auto& get(tuple<Ts...> const& ts) noexcept {
    return (static_cast<details::tuple_impl<Index, tuple_element_t<Index, Ts...>> const&>(ts))
        .get();
}

template <size_t Index, typename... Ts>
constexpr const auto&& get(tuple<Ts...> const&& ts) noexcept {
    return (static_cast<details::tuple_impl<Index, tuple_element_t<Index, Ts...>> const&&>(ts))
        .get();
}

// template deduction guideline
template <typename... Ts>
tuple(Ts... elems) -> tuple<Ts...>;

template <size_t Index, typename... Ts, typename... Us>
bool tuple_compare(tuple<Ts...> const& t1, tuple<Us...> const& t2) {
    if constexpr (Index == 0) {
        return get<0>(t1) == get<0>(t2);
    } else {
        return get<Index>(t1) == get<Index>(t2) && tuple_compare<Index - 1>(t1, t2);
    }
}

template <typename... Ts, typename... Us>
bool operator==(tuple<Ts...> const& t1, tuple<Us...> const& t2) {
    static_assert(sizeof...(Ts) == sizeof...(Us),
                  "tuple objects can only be compared if they have equal sizes");
    return tuple_compare<sizeof...(Ts) - 1>(t1, t2);
}

template <typename... Ts, typename... Us>
bool operator!=(tuple<Ts...> const& t1, tuple<Us...> const& t2) {
    return !(t1 == t2);
}

}  // namespace std
