/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stddef.h>
#include <stdint.h>

#include <new>

export module std.vector;

export import std.initializer_list;
export import std.type_traits;

// placement new prototypes
void* operator new(size_t size, void* ptr);
void* operator new[](size_t size, void* ptr);

export namespace std {

template <typename T>
class vector {
 public:
    using value_type = T;
    using iterator = T*;
    using const_iterator = T const*;
    using reference = value_type&;

    constexpr vector() : s(0), c(0), buf(nullptr), a(nullptr) {}
    constexpr vector(const std::initializer_list<T>& il) : vector() {
        reserve(il.size());
        for (auto&& t : il)
            push_back(t);
    }
    constexpr vector(const vector& vec) : vector() {
        for (auto& e : vec)
            push_back(e);
    }
    constexpr vector& operator=(const vector& vec) {
        resize(0);
        for (const auto& e : vec)
            push_back(e);
        return *this;
    }
    constexpr vector& operator=(const std::initializer_list<T>& il) {
        // call destructor for all objects
        for (auto p = a; p != &a[s]; ++p)
            p->~T();
        s = 0;
        reserve(il.size());
        for (auto&& t : il)
            push_back(t);
        return *this;
    }
    constexpr ~vector() {
        // call destructor for all objects
        for (auto p = a; p != &a[s]; ++p)
            p->~T();
        delete[] buf;  // delete memory
    }
    constexpr size_t capacity() const noexcept { return c; }
    constexpr size_t size() const noexcept { return s; }

    constexpr void reserve(size_t n) {
        if (c < n) {
            c = n;
            align_u8* tmp_buf = new align_u8[c];
            T* p = reinterpret_cast<T*>(tmp_buf);

            // copy objects to new location
            for (size_t i = 0; i != s; ++i)
                new (&p[i]) T(a[i]);
            // destroy previous objects
            for (size_t i = 0; i != s; ++i)
                a[i].~T();

            delete[] buf;  // delete old buffer

            a = p;
            buf = tmp_buf;
        }
    }

    constexpr void resize(size_t n) {
        if (n == s)
            return;

        if (n < s) {
            for (auto p = &a[n]; p != &a[s]; ++p)
                p->~T();
            s = n;
        } else if (n > s) {
            reserve(n);
            while (s < n)
                new (&a[s++]) T{};
        }
    }

    constexpr void clear() noexcept {
        // call destructor for all objects
        for (auto p = a; p != &a[s]; ++p)
            p->~T();
        s = 0;
    }

    //
    // push_back - Add new object to the end
    //
    constexpr void push_back(T const& v) {
        alloc_for_size(s + 1);
        new (&a[s++]) T{v};
    }

    //
    // push_back - Add new object to the end (rvalue)
    //
    constexpr void push_back(T&& v) {
        alloc_for_size(s + 1);
        new (&a[s++]) T{static_cast<T&&>(v)};
    }

    template <typename... Args>
    constexpr reference emplace_back(Args&&... args) {
        alloc_for_size(s + 1);
        auto elem_p = new (&a[s++]) T{std::forward<Args>(args)...};
        return *elem_p;
    }

    //
    // erase - Erase one element from the vector
    //
    // It will move all objects in case of a gap
    //
    constexpr iterator erase(const_iterator pos) {
        if (pos >= end())
            return end();

        for (iterator p = const_cast<iterator>(pos) + 1; p < end(); ++p)
            *(p - 1) = std::move(*p);
        pos->~T();
        s--;
        return const_cast<iterator>(pos);
    }

    //
    // insert - Insert new T object
    //
    // @pos     Iterator (position) where the new element will be inserted
    // @val     New T object
    //
    constexpr iterator insert(const_iterator pos, T const& val) {
        alloc_for_size(s + 1);
        for (iterator p = end(); p != pos; --p)
            *p = std::move(p[-1]);
        s++;
        iterator p = const_cast<iterator>(pos);
        new (p) T{val};
        return p;
    }

    //
    // insert - Insert new T object (rvalue)
    //
    // @pos     Iterator (position) where the new element will be inserted
    // @val     New T object
    //
    constexpr iterator insert(const_iterator pos, T&& val) {
        alloc_for_size(s + 1);
        for (iterator p = end(); p != pos; --p)
            *p = std::move(*(p - 1));
        s++;
        iterator p = const_cast<iterator>(pos);
        new (p) T{std::move(val)};
        return p;
    }

    template <typename... Args>
    constexpr iterator emplace(const_iterator pos, Args&&... args) {
        alloc_for_size(s + 1);
        for (iterator p = end(); p != pos; --p)
            *p = std::move(*(p - 1));
        s++;
        iterator p = const_cast<iterator>(pos);
        new (p) T{std::forward<Args>(args)...};
        return p;
    }

    //
    // data - Returns a pointer to the first element in the array.
    //
    constexpr T* data() noexcept { return a; }
    constexpr T const* data() const noexcept { return a; }

    [[nodiscard]] constexpr bool empty() const noexcept { return s == 0; }

    // iterators
    constexpr iterator begin() noexcept { return &a[0]; }
    constexpr iterator end() noexcept { return &a[s]; }
    // const iterators
    constexpr const_iterator begin() const noexcept { return &a[0]; }
    constexpr const_iterator end() const noexcept { return &a[s]; }

    constexpr T& operator[](size_t i) { return a[i]; }
    constexpr T const& operator[](size_t i) const { return a[i]; }

 private:
    // create a new type which is compliant with T alignment
    struct alignas(T) align_u8 {
        uint8_t bytes[sizeof(T)];
    };

    // capacity when inserting the first element, since most likely we will keep inserting more
    static constexpr size_t INITIAL_CAP = 4;

    size_t s;       // size
    size_t c;       // capacity
    align_u8* buf;  // raw memory
    T* a;           // Array of T objects

    //
    // alloc_for_size - make sure we have capacity for new size
    //
    // If new size is bigger than capacity it will allocate more memory and move objects
    //
    // @new_size    New size (number of T objects) requested
    //
    void alloc_for_size(size_t new_size) {
        if (c < new_size) {
            if (INITIAL_CAP > new_size) {
                reserve(INITIAL_CAP);
            } else {
                reserve(new_size * 2);
            }
        }
    }
};

// TODO: Implement optimized version for standard layout types

}  // namespace std
