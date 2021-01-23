/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

/*
 * exception module defines class exception. This is the only exception kind is allowed to throw,
 * since the exception support is limited because of lack of RTTI it should never used this as a
 * base for a derived class. Instead, use error code in case you need to catch an specific kind of
 * error or a builtin C++ type (discouraged).
 */

module;

#include <errcodes.h>

export module lib.exception;

export import std.string;

export namespace sc::lib {

class exception {
 public:
    exception(std::string const& s, int code = ERR_GENERIC) : str(s), code(code) {}
    std::string const& msg() const { return str; }
    int error() { return code; }

 private:
    std::string str;
    int code;
};

}  // namespace sc::lib
