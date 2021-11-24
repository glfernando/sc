/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stddef.h>

export module lib.async;

export import std.tuple;
export import std.string;
import std.type_traits;
export import std.utility;
export import core.thread;
import lib.fmt;

using lib::fmt::println;

export namespace lib {

using core::thread::thread_t;

template <class T>
std::decay_t<T> decay_copy(T&& v) {
    return std::forward<T>(v);
}

template <typename F, typename... Args>
class async {
    using R = std::invoke_result_t<F, Args...>;

 public:
    async(std::string const& name, unsigned affinity, F&& f, Args&&... args)
        : func{decay_copy(std::forward<F>(f))},
          args{decay_copy(std::forward<Args>(args))...},
          t{name,
            [](void* data) {
                auto self = reinterpret_cast<async*>(data);
                self->call();
            },
            this, affinity} {}

    async(unsigned affinity, F&& f, Args&&... args)
        : async("async", affinity, std::forward<F>(f), std::forward<Args>(args)...) {}

    async(std::string const& name, F&& f, Args&&... args)
        : async(name, core::thread::AFFINITY_ALL, std::forward<F>(f), std::forward<Args>(args)...) {
    }

    async(F&& f, Args&&... args)
        : async("async", core::thread::AFFINITY_ALL, std::forward<F>(f),
                std::forward<Args>(args)...) {}

    auto wait_for_result() {
        t.join();
        return res.get_result();
    }

 private:
    template <typename R>
    class async_result {
     public:
        void set_result(R const&& r) { res = r; }
        R const& get_result() const { return res; }
        R& get_result() { return res; }

     private:
        R res;
    };

    template <>
    class async_result<void> {
     public:
        void set_result(void) {}
        void get_result(void) {}
    };

    void call() {
        if constexpr (!std::is_same<R, void>::value) {
            res.set_result(real_call(std::make_index_sequence<sizeof...(Args)>{}));
        } else {
            real_call(std::make_index_sequence<sizeof...(Args)>{});
        }
    }

    template <size_t... I>
    R real_call(std::index_sequence<I...>) {
        return func(std::get<I>(args)...);
    }

    // variables
    std::decay_t<F> func;
    std::tuple<std::decay_t<Args>...> args;
    async_result<R> res;
    thread_t t;
};

// deduction guides
template <typename F, typename... Args>
async(std::string const& name, unsigned affinity, F&&, Args&&... args) -> async<F, Args...>;

template <typename F, typename... Args>
async(std::string const&& name, unsigned affinity, F&&, Args&&... args) -> async<F, Args...>;

template <typename F, typename... Args>
async(const char* name, unsigned affinity, F&&, Args&&... args) -> async<F, Args...>;

template <typename F, typename... Args>
async(std::string const& name, F&&, Args&&... args) -> async<F, Args...>;

template <typename F, typename... Args>
async(std::string const&& name, F&&, Args&&... args) -> async<F, Args...>;

template <typename F, typename... Args>
async(const char* name, F&&, Args&&... args) -> async<F, Args...>;

template <typename F, typename... Args>
async(unsigned affinity, F&&, Args&&... args) -> async<F, Args...>;

template <typename F, typename... Args>
async(F&&, Args&&... args) -> async<F, Args...>;

}  // namespace lib
