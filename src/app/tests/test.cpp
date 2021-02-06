/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

#include <app/shell.h>
#include <errcodes.h>
#include <stddef.h>
#include <test.h>

import lib.fmt;
import std.string;
import lib.exception;

using sc::lib::exception::exception;
using sc::lib::fmt::print;
using sc::lib::fmt::println;
using std::string;

void cmd_test_usage() {
    println("test <name> [test args]...");
}

class test_list {
 public:
    using it = test_data*;
    test_list(it start, it end) : start(start), end_(end) {}
    it begin() { return start; }
    it end() { return end_; }
    test_data& operator[](int index) { return start[index]; }
    size_t size() const noexcept { return end_ - start; }

 private:
    it start;
    it end_;
};

static test_list tests(__tests_start, __tests_end);

void list_tests() {
    for (auto& test : tests)
        println("- {}.{}", test.group, test.name);
}

test_data& find_test(string const& group, string const& name) {
    for (auto& test : tests)
        if (group == test.group && name == test.name)
            return test;
    throw exception("", ERR_NOT_FOUND);
}

#define COLOR_DEFAULT "\e[39m"
#define COLOR_RED     "\e[31m"
#define COLOR_GREEN   "\e[32m"

struct test_results {
    size_t total;
    size_t success;
    size_t fail;
};

void show_results(test_results res) {
    auto [total, success, fail] = res;

    println("\n=============================================================");
    println("total tests {}, sucess {}, failed {}, skipped {}", total, success, fail,
            total - success - fail);
    if (fail) {
        println(COLOR_RED "[  ERROR   ]" COLOR_DEFAULT " some tests failed");
    } else {
        println(COLOR_GREEN "[    OK    ]" COLOR_DEFAULT " no test failed");
    }
    println("=============================================================");
}

int run_test(test_data& test) {
    int ret = 0;
    try {
        test.func();
        print(COLOR_GREEN "[  PASSED  ]");
    } catch (exception& e) {
        println("{}", e.msg());
        print(COLOR_RED "[  FAILED  ]");
        ret = e.error();
    } catch (...) {
        println("unexpected exception\n");
        // we should never reach here, but just to be safe
        print(COLOR_RED "[  FAILED  ]");
        // unexpected exception, tell to stop
        ret = ERR_TEST_ASSERT;
    }
    print(COLOR_DEFAULT);
    println(" {}.{}", test.group, test.name);
    return ret;
}

test_results run_all() {
    size_t fail = 0;
    size_t success = 0;
    for (auto& test : tests) {
        int ret = run_test(test);
        if (!ret) {
            success++;
        } else {
            fail++;
            if (ret == ERR_TEST_ASSERT)
                break;
        }
    }
    return {tests.size(), success, fail};
}

test_results run_test_group(string const& group) {
    size_t total = 0;
    size_t fail = 0;
    size_t success = 0;
    bool skip = false;

    for (auto& test : tests) {
        if (test.group != group)
            continue;
        total++;

        if (skip)
            continue;

        int ret = run_test(test);
        if (!ret) {
            success++;
        } else {
            fail++;
            if (ret == ERR_TEST_ASSERT)
                skip = true;
        }
    }

    return {total, success, fail};
}

static int cmd_test(int argc, char const* argv[]) {
    if (argc < 2) {
        cmd_test_usage();
        return ERR_INVALID_ARGS;
    }

    string arg = argv[1];
    if (arg == "list") {
        list_tests();
        return 0;
    }

    string& group = arg;
    if (argc == 3) {
        // specific test
        string name = argv[2];
        try {
            auto& test = find_test(group, name);
            return run_test(test);
        } catch (...) {
            println("test \"{}.{}\" not found", group, name);
            return ERR_NOT_FOUND;
        }
    }

    test_results res;
    if (arg == "all") {
        // special group "all"
        res = run_all();
    } else {
        res = run_test_group(group);
    }

    if (res.total == 0) {
        println("no tests found for group {}", group);
        return;
    }

    show_results(res);

    // result error if any test failed. This can be used for validation
    return res.fail ? ERR_INVALID : 0;
}

shell_declare_static_cmd(test, "run a test", cmd_test, cmd_test_usage);
