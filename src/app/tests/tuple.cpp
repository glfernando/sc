/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

#include <test.h>

import std.tuple;
import std.string;
import lib.fmt;
import lib.exception;

using sc::lib::fmt::println;
using std::string;
using std::tuple;

TEST(tuple, get) {
    tuple t(1, 10, string("hello"), 'a');

    EXPECT(std::get<0>(t) == 1);
    EXPECT(std::get<1>(t) == 10);
    EXPECT(std::get<2>(t) == "hello");
    EXPECT(std::get<3>(t) == 'a');

    // check tuple elem size is 4
    EXPECT(std::tuple_size<decltype(t)>::value == 4);
}

TEST(tuple, empty) {
    // emtpy tuple should be size 1
    tuple<> t0;
    EXPECT(sizeof(t0) == 1);
}

TEST(tuple, write) {
    tuple t(1, 2, string("hello"));

    std::get<0>(t) = 10;
    std::get<1>(t) = 12;
    std::get<2>(t) += " world";

    EXPECT(std::get<0>(t) == 10);
    EXPECT(std::get<1>(t) == 12);
    EXPECT(std::get<2>(t) == "hello world");
}

struct Point {
    int x;
    int y;
};

bool operator==(Point const& p1, Point const& p2) {
    return p1.x == p2.x && p1.y == p2.y;
}

bool operator!=(Point const& p1, Point const& p2) {
    return !(p1 == p2);
}

TEST(tuple, compare) {
    tuple t1(1, 2, 3);
    tuple t2(2, 2, 3);

    EXPECT(t1 != t2);

    t2 = t1;
    EXPECT(t1 == t2);

    tuple t3(1L, 2, 3);
    EXPECT(t1 == t3);

    // let's compare user define types
    tuple t4(Point{10, 11}, string("hello"));
    tuple t5(Point{10, 11}, string("hello"));
    tuple t6(Point{10, 11}, string("helloo"));

    EXPECT(t4 == t5);
    EXPECT(t5 != t6);
}
