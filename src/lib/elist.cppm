/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module lib.elist;

//
// Embedded list or elist is used when the node is part of the class, so that just can insert or
// move from list to list without any copy or move, it is just like a simple C liked list
// implementation
//

#include <stddef.h>

import std.type_traits;

export namespace lib {

struct elist_node {
    elist_node* next;
    elist_node* prev;
};

template <typename T>
concept ElistElement = std::is_base_of_v<elist_node, T>;

template <ElistElement E>
class elist : public lib::elist_node {
    using N = lib::elist_node;

 public:
    constexpr elist() noexcept {
        next = this;
        prev = this;
    }
    constexpr bool empty() const noexcept { return next == this; }

    constexpr void add_tail(E& e) noexcept { add(&e, prev, this); }
    constexpr void add_tail(E* e) noexcept { add_tail(*e); }

    constexpr void add_head(E& e) noexcept { add(&e, this, next); }
    constexpr void add_head(E* e) noexcept { add_head(*e); }

    constexpr void add_after(E& e, E& after) noexcept { add(&e, after, after.next); }
    constexpr void add_after(E* e, E* after) noexcept { add_afer(*e, *after); }

    constexpr void add_before(E& e, E& before) noexcept { add(&e, before.prev, before); }
    constexpr void add_before(E* e, E* before) noexcept { add_afer(*e, *before); }

    constexpr void remove(E& e) noexcept {
        if (e.next == nullptr || e.prev == nullptr) {
            // TODO: consider trowing an exception
            return;
        }

        e.next->prev = e.prev;
        e.prev->next = e.next;
        e.next = e.prev = nullptr;
    }

    constexpr void remove(E* e) noexcept { remove(*e); }

    constexpr size_t size() const noexcept {
        // TODO: maybe a size variable?
        size_t size = 0;
        for (auto i = begin(); i != end(); ++i)
            ++size;
        return size;
    }

    struct iterator {
        elist_node* e;

        bool operator==(iterator const& other) const { return e == other.e; }
        bool operator!=(iterator const& other) const { return !(*this == other); }

        E& operator*() const { return *static_cast<E*>(e); }

        iterator& operator++() {
            e = e->next;
            return *this;
        }
    };

    iterator begin() const { return iterator{next}; }

    iterator end() const { return iterator{prev->next}; }

 private:
    constexpr void add(N* e, N* prev, N* next) noexcept {
        next->prev = e;
        e->next = next;
        e->prev = prev;
        prev->next = e;
    }
};

}  // namespace lib
