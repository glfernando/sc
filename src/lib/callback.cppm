/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

// simple helper to create a callback whch accepts any parameters and can be a function pointer or a
// lamba

export module lib.callback;

import std.tuple;

using std::tuple;

namespace {

template <typename T>
std::decay_t<T> decay_copy(T&& v) {
    return std::forward<T>(v);
}

template <typename F, typename... Args>
class callback_wrapper;

}

export namespace lib {

template <typename... Args>
class callback {
 public:
    virtual void operator()(Args... args) = 0;
    virtual ~callback() {}

    template <typename F>
    static callback* create(F&& f) {
        return new callback_wrapper<F, Args...>(std::forward<F>(f));
    }
};

}

namespace {

template <typename F, typename... Args>
class callback_wrapper : public lib::callback<Args...> {
 public:
    callback_wrapper(F&& f) : func{decay_copy(std::forward<F>(f))} {}

    void operator()(Args... args) override { return func(args...); }

 private:
    std::decay_t<F> func;
};

}
