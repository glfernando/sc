/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module lib.equeue;

import lib.elist;
import lib.exception;

using lib::exception;

export namespace lib {

template <typename T>
class equeue : public elist<T> {
 public:
    void push(T& t) { this->add_tail(t); }

    void push(T* t) { push(*t); }

    // TODO: nodiscard ?
    constexpr T& pop_ref() {
        if (this->empty()) {
            throw exception("empty queue");
        }

        T* t = static_cast<T*>(this->next);
        this->remove(t);
        return *t;
    }

    // TODO: nodiscard ?
    constexpr T* pop() {
        if (this->empty()) {
            return nullptr;
        }

        T* t = static_cast<T*>(this->next);
        this->remove(t);
        return t;
    }
};

}  // namespace lib
