/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

#include <errcodes.h>
#include <stddef.h>
#include <stdint.h>
#include <test.h>

import std.vector;
using std::vector;

import lib.fmt;
using lib::fmt::println;

struct foo {
    foo(int x = 0) : val(x) {
        counter++;
        max_counter++;
    }
    foo(foo const&) {
        counter++;
        max_counter++;
    }
    foo(foo&&) {
        counter++;
        max_counter++;
    }
    ~foo() { counter--; }
    int val;
    // current "alive"
    inline static int counter;
    inline static int max_counter;
};

TEST(vector, lifetime) {
    {
        vector<foo> vec;
        EXPECT(foo::counter == 0);

        vec.push_back(foo());
        EXPECT(foo::counter == 1);
    }

    EXPECT(foo::counter == 0);
}

TEST(vector, size) {
    vector vec = {1, 2, 3, 4};
    EXPECT(4 == vec.size());
    vec.push_back(5);
    vec.push_back(6);
    EXPECT(6 == vec.size());
}

static bool is_addr_algined(void* ptr, size_t align) {
    uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
    return !(addr & (align - 1));
}

TEST(vector, alignment) {
    constexpr int TEST_ALIGNMENT = 128;
    struct alignas(TEST_ALIGNMENT) aligned_data {
        int a;
    };
    vector<aligned_data> vec;
    vec.push_back(aligned_data{});
    vec.push_back(aligned_data{});
    vec.push_back(aligned_data{});
    vec.push_back(aligned_data{});

    // make sure each element has correct alignment
    for (auto& data : vec) {
        ASSERT(is_addr_algined(&data, TEST_ALIGNMENT));
    }
}

TEST(vector, emplace) {
    struct Point {
        int x;
        int y;
        int z;
        Point(int x, int y, int z) : x(x), y(y), z(z) {}
    };

    vector<Point> vec;
    auto& e = vec.emplace_back(2, 4, 5);
    EXPECT(e.x == 2 && e.y == 4 && e.z == 5);

    auto e1 = vec.emplace(vec.begin(), 6, 7, 8);
    EXPECT(e1 == vec.begin());
    EXPECT(e1->x == 6 && e1->y == 7 && e1->z == 8);
}

TEST(vector, erase) {
    vector<int> vec;
    for (int i = 0; i < 100; ++i) {
        vec.push_back(i);
        if (i == 0 || i == 50 || i == 99)
            vec.push_back(i);
    }

    // erase first element
    vec.erase(vec.begin());
    // erase at the middle
    vec.erase(vec.begin() + 50);
    // erase last element
    vec.erase(vec.end() - 1);

    // verify vector values
    for (int i = 0; i < 100; ++i) {
        EXPECT(i == vec[i]);
    }
};
