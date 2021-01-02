/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (C) 2020 Fernando Lugo <lugo.fernando@gmail.com>.
 */

module;

#include <stdio.h>
#include <string.h>

export module std.string;

import lib.heap;

using sc::lib::heap::alloc;
using sc::lib::heap::free;

export namespace std {

template <typename C>
class basic_string {
 public:
    basic_string();
    basic_string(const C* p);
    basic_string(const basic_string&);
    basic_string& operator=(const basic_string&);
    basic_string(basic_string&&);
    basic_string& operator=(basic_string&&);
    ~basic_string();

    C& operator[](int n) { return ptr[n]; }
    C operator[](int n) const { return ptr[n]; }

    basic_string& operator+=(C c);
    basic_string& operator+=(const C* p);
    C* c_str() { return ptr; }
    const C* c_str() const { return ptr; }

    int size() const { return sz; }
    int capacity() const { return sz <= short_max ? short_max : sz + space; }
    bool empty() const { return sz == 0; }

 private:
    static const int short_max = 15;
    int sz;
    C* ptr;
    union {
        int space;
        C ch[short_max + 1];
    };
    static int string_len(const C* s)
    {
        const C* t = s;
        while (*t)
            t++;
        return t - s;
    }
    static void string_copy(C* dst, const C* src)
    {
        while ((*dst++ = *src++)) {}
    }

    static C* expand(const C* ptr, int n)
    {
        C* p = static_cast<C*>(alloc(sizeof(C) * n, alignof(C)));
        string_copy(p, ptr);
        return p;
    }

    void copy_from(const basic_string& s);
    void move_from(basic_string& s);
};

//
// Implementation
//
template <typename C>
void basic_string<C>::copy_from(const basic_string& s)
{
    if (s.sz <= short_max) {
        memcpy(this, &s, sizeof(s));
        ptr = ch;
    } else {
        ptr = expand(s.ptr, s.sz + 1);
        sz = s.sz;
        space = 0;
    }
}

template <typename C>
void basic_string<C>::move_from(basic_string& s)
{
    if (s.sz <= short_max) {
        memcpy(this, &s, sizeof(s));
        ptr = ch;
    } else {
        ptr = s.ptr;
        sz = s.sz;
        space = s.space;
        s.ptr = s.ch;
        s.sz = 0;
        s.ch[0] = 0;
    }
}

template <typename C>
basic_string<C>::basic_string() : sz{0}, ptr{ch}
{
    ch[0] = 0;
}

template <typename C>
basic_string<C>::basic_string(const C* p)
    : sz{string_len(p)},
      ptr{sz <= short_max ? ch : static_cast<C*>(alloc(sizeof(C) * (sz + 1), alignof(C)))},
      space{0}
{
    string_copy(ptr, p);
}

template <typename C>
basic_string<C>::basic_string(const basic_string& s)
{
    copy_from(s);
}

template <typename C>
basic_string<C>::basic_string(basic_string&& s)
{
    move_from(s);
}

template <typename C>
basic_string<C>& basic_string<C>::operator=(const basic_string& s)
{
    if (this == &s)
        return *this;
    C* p = (short_max < sz) ? ptr : nullptr;
    copy_from(s);
    free(p);
    return *this;
}

template <typename C>
basic_string<C>& basic_string<C>::operator=(basic_string&& s)
{
    if (this == &s)
        return *this;
    if (short_max < sz)
        free(ptr);
    move_from(s);
    return *this;
}

template <typename C>
basic_string<C>& basic_string<C>::operator+=(C c)
{
    if (sz == short_max) {
        int n = sz + sz + 2;
        ptr = expand(ptr, n);
        space = n - sz - 2;
    } else if (short_max < sz) {
        if (space == 0) {
            int n = sz + sz + 2;
            C* p = expand(ptr, n);
            free(ptr);
            ptr = p;
            space = n - sz - 2;
        } else {
            --space;
        }
    }
    ptr[sz] = c;
    ptr[++sz] = 0;

    return *this;
}

template <typename C>
basic_string<C>& basic_string<C>::operator+=(const C* p)
{
    for (int i = 0; p[i]; ++p)
        *this += p[i];
    return *this;
}

template <typename C>
basic_string<C>::~basic_string()
{
    if (short_max < sz)
        free(ptr);
}

template <typename C>
basic_string<C>& operator+=(basic_string<C>& s1, const basic_string<C>& s2)
{
    for (auto c : s2)
        s1 += c;
    return s1;
}

template <typename C>
basic_string<C> operator+(const basic_string<C>& s1, const basic_string<C>& s2)
{
    basic_string<C> s{s1};
    s += s2;
    return s;
}

template <typename C>
basic_string<C> operator+(const basic_string<C>& s1, const C* p)
{
    basic_string<C> s{s1};
    s += basic_string<C>(p);
    return s;
}

template <typename C>
basic_string<C> operator+(const C* p, const basic_string<C>& s1)
{
    basic_string<C> s{p};
    s += s1;
    return s;
}

template <typename C>
bool operator==(const basic_string<C>& s1, const basic_string<C>& s2)
{
    if (s1.size() != s2.size())
        return false;

    for (int i = 0; i != s1.size(); ++i)
        if (s1[i] != s2[i])
            return false;
    return true;
}

template <typename C>
bool operator!=(const basic_string<C>& s1, const basic_string<C>& s2)
{
    return !(s1 == s2);
}

template <typename C>
bool operator==(const basic_string<C>& str, const C* cstr)
{
    const C* tmp = str.c_str();
    for (; *tmp && *tmp == *cstr; ++tmp, ++cstr) {}

    return *tmp == *cstr;
}

template <typename C>
bool operator!=(const basic_string<C>& str, const C* cstr)
{
    return !(str == cstr);
}

template <typename C>
bool operator==(const C* cstr, const basic_string<C>& str)
{
    const C* tmp = str.c_str();
    for (; *tmp && *tmp == *cstr; ++tmp, ++cstr) {}

    return *tmp == *cstr;
}

template <typename C>
bool operator!=(const C* cstr, const basic_string<C>& str)
{
    return !(str == cstr);
}

template <typename C>
C* begin(basic_string<C>& s)
{
    return s.c_str();
}

template <typename C>
C* end(basic_string<C>& s)
{
    return s.c_str() + s.size();
}

template <typename C>
const C* begin(const basic_string<C>& s)
{
    return s.c_str();
}

template <typename C>
const C* end(const basic_string<C>& s)
{
    return s.c_str() + s.size();
}

typedef basic_string<char> string;
typedef basic_string<wchar_t> wstring;
typedef basic_string<char16_t> u16string;

}  // namespace std

static const int BUF_MAX = 22;  // enough to hold ULLONG_MAX

export namespace std {

// helper functions
string to_string(int val)
{
    char buf[BUF_MAX];
    snprintf(buf, sizeof buf, "%d", val);
    return string{buf};
}

string to_string(long val)
{
    char buf[BUF_MAX];
    snprintf(buf, sizeof buf, "%ld", val);
    return string{buf};
}

string to_string(long long val)
{
    char buf[BUF_MAX];
    snprintf(buf, sizeof buf, "%lld", val);
    return string{buf};
}

string to_string(unsigned val)
{
    char buf[BUF_MAX];
    snprintf(buf, sizeof buf, "%u", val);
    return string{buf};
}

string to_string(unsigned long val)
{
    char buf[BUF_MAX];
    snprintf(buf, sizeof buf, "%lu", val);
    return string{buf};
}

string to_string(unsigned long long val)
{
    char buf[BUF_MAX];
    snprintf(buf, sizeof buf, "%llu", val);
    return string{buf};
}

}  // namespace std
