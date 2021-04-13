/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

#include <test.h>

import lib.exception;
import lib.fmt;

using lib::exception;

class my_except {};

TEST(except, throw) {
    // throw fundamental type
    try {
        throw 10;
    } catch (int i) { EXPECT(i == 10); }

    // throw pointer
    try {
        throw new int{15};
    } catch (int i) {
        // this should not be caught
        EXPECT(false);
    } catch (int*& i) {
        EXPECT((*i) == 15);
        delete i;
    }

    // throw a class
    try {
        throw my_except{};
    } catch (my_except const&) {
    } catch (...) {
        // we should not here
        EXPECT(false);
    }

    // throw a lib exception
    try {
        throw exception("real exception");
    } catch (exception& e) { EXPECT(e.msg() == "real exception"); }

    // throw and catch all
    try {
        throw 12.5;
    } catch (exception& e) { EXPECT(e.msg() == "we should not be here"); } catch (...) {
        EXPECT(true);
    }

    // retrow a new exception
    /* TODO: fix
    try {
        try {
            throw 10;
        } catch (int i) {
            EXPECT(i == 0);
            throw exception("throw new one");
        }
    } catch (exception& e) {
        EXPECT(e.msg() == "throw new one");
    } catch (...) {
        EXPECT(false);
    }*/

    // retrow
    int count = 0;
    try {
        try {
            throw int{5};
        } catch (int x) {
            count += x;
            throw;
        }
    } catch (int i) { EXPECT(i == 5); }
}

struct foo {
    foo() { counter++; }
    ~foo() { counter--; }

    static int counter;
};

int foo::counter = 0;

TEST(except, cleanup) {
    try {
        foo f1;
        EXPECT(foo::counter == 1);
        {
            foo f2;
            EXPECT(foo::counter == 2);
        }
        foo f3;
        EXPECT(foo::counter == 2);
        throw exception("f1 will be cleaned up");
        foo f4;
        EXPECT(false);
    } catch (exception& e) { EXPECT(foo::counter == 0); }
}
