/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

#pragma once

import std.string;
import lib.fmt;
import lib.exception;

struct test_data {
    char const* group;
    char const* name;
    void (*func)(void);
};

extern test_data __tests_start[];
extern test_data __tests_end[];

#define ERR_TEST_BASE   (-0x54534554)  // "TEST"
#define ERR_TEST_EXPECT (ERR_TEST_BASE - 1)
#define ERR_TEST_ASSERT (ERR_TEST_BASE - 2)

#define TEST(group_, name_)                                                           \
    static void group_##name_(void);                                                  \
    static const test_data test_##group_##name_ __attribute__((section(".tests." #group_ #name_))) \
        __attribute__((used)) = {                                                     \
            .group = #group_,                                                         \
            .name = #name_,                                                           \
            .func = group_##name_,                                                    \
    };                                                                                \
    static void group_##name_(void)

#define EXPECT(expr)                                                                             \
    if (!(expr)) {                                                                               \
        using lib::fmt::sprint;                                                                  \
        std::string msg = sprint("{}:{}: except \"{}\" failed", __FILE_NAME__, __LINE__, #expr); \
        throw lib::exception(msg, ERR_TEST_EXPECT);                                              \
    }

#define ASSERT(expr)                                                                             \
    if (!(expr)) {                                                                               \
        using lib::fmt::sprint;                                                                  \
        std::string msg = sprint("{}:{}: assert \"{}\" failed", __FILE_NAME__, __LINE__, #expr); \
        throw lib::exception(msg, ERR_TEST_ASSERT);                                              \
    }
