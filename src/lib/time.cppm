/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module lib.time;

import lib.timestamp;
import std.string;
import std.concepts;

using lib::timestamp::freq;

namespace lib::time {

template <unsigned long Num, unsigned long Denom = 1>
class ratio {
 public:
    static constexpr unsigned long num = Num;
    static constexpr unsigned long den = Denom;
};

using nano = ratio<1, 1000000000>;
using micro = ratio<1, 1000000>;
using milli = ratio<1, 1000>;
using minute = ratio<60>;
using hour = ratio<60 * 60>;

}  // namespace lib::time

export namespace lib::time {

template <typename Factor = ratio<1>>
class time_t {
 public:
    constexpr time_t(unsigned long val) : val(val) {}
    template <typename F>
    constexpr time_t(time_t<F> const& t2) {
        val = t2.count() * Factor::den * F::num / Factor::num / F::den;
    }
    unsigned long count() const { return val; }
    unsigned long ticks() const { return val * freq() * Factor::num / Factor::den; }

    // TODO: make it more flexible
    template <typename F>
    time_t operator+(time_t<F> const& t) const {
        return time_t(val + t.count() * Factor::den * F::num / Factor::num / F::den);
    }
    template <std::integral I>
    time_t operator+(I x) const {
        return time_t(val + x);
    }
    template <std::integral I>
    time_t operator*(I x) const {
        return time_t(val * x);
    }

    // helper operator to easy printing
    operator std::string() const {
        // TODO: add suffix based on ratio
        return std::to_string(val);
    }

    unsigned long get_val() const noexcept { return val; }

 private:
    unsigned long val;
};

// helper types
using time_ns_t = time_t<nano>;
using time_us_t = time_t<micro>;
using time_ms_t = time_t<milli>;
using time_s_t = time_t<>;
using time_min_t = time_t<minute>;
using time_h_t = time_t<hour>;

constexpr time_ns_t operator""ns(unsigned long long t_ns) {
    return time_ns_t(t_ns);
}

constexpr time_us_t operator""us(unsigned long long t_us) {
    return time_us_t(t_us);
}

constexpr time_ms_t operator""ms(unsigned long long t_ms) {
    return time_ms_t(t_ms);
}

constexpr time_s_t operator""s(unsigned long long t) {
    return time_s_t(t);
}

constexpr time_min_t operator""min(unsigned long long t) {
    return time_min_t(t);
}

constexpr time_h_t operator""h(unsigned long long t) {
    return time_h_t(t);
}

void delay(auto t) {
    auto to = lib::timestamp::ticks() + t.ticks();
    while (to > lib::timestamp::ticks()) {}
}

time_ns_t now() {
    // return in nano seconds so we don't lose accuracy
    return lib::timestamp::ns();
}

template <typename T>
bool operator==(time_t<T> const& t1, time_t<T> const& t2) {
    return t1.count() == t2.count();
}

template <typename T>
bool operator!=(time_t<T> const& t1, time_t<T> const& t2) {
    return !(t1 == t2);
}

template <typename T>
bool operator<(time_t<T> const& t1, time_t<T> const& t2) {
    return t1.count() < t2.count();
}

template <typename T>
bool operator<=(time_t<T> const& t1, time_t<T> const& t2) {
    return t1.count() <= t2.count();
}

template <typename T>
bool operator>(time_t<T> const& t1, time_t<T> const& t2) {
    return t1.count() > t2.count();
}

template <typename T>
bool operator>=(time_t<T> const& t1, time_t<T> const& t2) {
    return t1.count() >= t2.count();
}

template <typename T, typename U>
bool operator==(time_t<T> const& t1, time_t<U> const& t2) {
    if constexpr (T::den > U::den) {
        return t1.count() == time_t<T>(t2).count();
    } else if constexpr (T::den < U::den) {
        return time_t<U>(t1).count() == t2.count();
    } else {
        if constexpr (U::num > T::num)
            return t1.count() == time_t<T>(t2).count();
        else
            return time_t<U>(t1).count() == t2.count();
    }
}

template <typename T, typename U>
bool operator!=(time_t<T> const& t1, time_t<U> const& t2) {
    return !(t1 == t2);
}

template <typename T, typename U>
bool operator<(time_t<T> const& t1, time_t<U> const& t2) {
    if constexpr (T::den > U::den) {
        return t1.count() < time_t<T>(t2).count();
    } else if constexpr (T::den < U::den) {
        return time_t<U>(t1).count() < t2.count();
    } else {
        if constexpr (U::num > T::num)
            return t1.count() < time_t<T>(t2).count();
        else
            return time_t<U>(t1).count() < t2.count();
    }
}

template <typename T, typename U>
bool operator>(time_t<T> const& t1, time_t<U> const& t2) {
    if constexpr (T::den > U::den) {
        return t1.count() > time_t<T>(t2).count();
    } else if constexpr (T::den < U::den) {
        return time_t<U>(t1).count() > t2.count();
    } else {
        if constexpr (U::num > T::num)
            return t1.count() > time_t<T>(t2).count();
        else
            return time_t<U>(t1).count() > t2.count();
    }
}

template <typename T, typename U>
bool operator<=(time_t<T> const& t1, time_t<U> const& t2) {
    return !(t1 > t2);
}

template <typename T, typename U>
bool operator>=(time_t<T> const& t1, time_t<U> const& t2) {
    return !(t1 < t2);
}

}  // namespace lib::time
