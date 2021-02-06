/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module std.memory;

#include <stddef.h>

export namespace std {

typedef decltype(nullptr) nullptr_t;

template <typename T>
class unique_ptr {
 public:
    constexpr unique_ptr() noexcept : p(nullptr) {}
    constexpr unique_ptr(std::nullptr_t) noexcept : p(nullptr) {}
    explicit unique_ptr(T* p) noexcept : p(p) {}
    ~unique_ptr() { delete p; }

    unique_ptr(unique_ptr&& up) noexcept { swap(up); }

    unique_ptr& operator=(unique_ptr&& up) noexcept {
        swap(up);
        return *this;
    }

    // allow to assign nullptr
    unique_ptr& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    // delete copy constructor/assignment
    unique_ptr(unique_ptr const&) = delete;
    unique_ptr& operator=(unique_ptr const&) = delete;

    T* get() const noexcept { return p; }
    explicit operator bool() const noexcept { return p; }

    T& operator*() const noexcept { return *p; }
    T* operator->() const noexcept { return p; };

    void swap(unique_ptr& up) noexcept {
        T* tmp = p;
        p = up.p;
        up.p = tmp;
    }

    T* release() noexcept {
        T* tmp = p;
        p = nullptr;
        return tmp;
    }

    void reset(std::nullptr_t p = nullptr) noexcept { delete release(); }

    void reset(T* ptr) noexcept {
        delete release();
        p = ptr;
    }

 private:
    T* p;
};

// template specialization for arrays
template <typename T>
class unique_ptr<T[]> {
 public:
    constexpr unique_ptr() noexcept : p(nullptr) {}
    constexpr unique_ptr(std::nullptr_t) noexcept : p(nullptr) {}
    explicit unique_ptr(T* p) noexcept : p(p) {}
    ~unique_ptr() { delete[] p; }

    unique_ptr(unique_ptr&& up) noexcept { swap(up); }

    unique_ptr& operator=(unique_ptr&& up) noexcept {
        swap(up);
        return *this;
    }

    // allow to assign nullptr
    unique_ptr& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    // delete copy constructor/assignment
    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;

    T* get() const noexcept { return p; }
    explicit operator bool() const noexcept { return p; }

    T& operator[](size_t i) const { return p[i]; }

    void swap(unique_ptr& up) noexcept {
        T* tmp = p;
        p = up.p;
        up.p = tmp;
    }

    T* release() noexcept {
        T* tmp = p;
        p = nullptr;
        return tmp;
    }

    void reset(std::nullptr_t p = nullptr) noexcept { delete[] release(); }

    void reset(T* ptr) noexcept {
        delete[] release();
        p = ptr;
    }

 private:
    T* p;
};

template <typename T, typename U>
bool operator==(const unique_ptr<T>& p1, const unique_ptr<U>& p2) noexcept {
    return p1.get() == p2.get();
}

template <typename T, typename U>
bool operator!=(const unique_ptr<T>& p1, const unique_ptr<U>& p2) noexcept {
    return !(p1 == p2);
}

template <typename T>
bool operator==(const unique_ptr<T>& up, std::nullptr_t) noexcept {
    return up.get() == nullptr;
}

template <typename T>
bool operator!=(const unique_ptr<T>& up, std::nullptr_t) noexcept {
    return !(up == nullptr);
}

template <typename T>
bool operator==(std::nullptr_t, const unique_ptr<T>& up) noexcept {
    return nullptr == up.get();
}

template <typename T>
bool operator!=(std::nullptr_t, const unique_ptr<T>& up) noexcept {
    return !(nullptr == up);
}

}  // namespace std
