/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

/*
 * device module:
 *
 * This are the rules all devices need to follow
 * - constructor(): don't do any device initialization, the only job it does is to read platform
 *   data and save it for later
 * - init(): do device initialization. After this function is executed device should be ready to
 *   accept user inputs. Most likely devices need to consider multithread scenarios
 * - deinit(): device that are removable needs to implement this function in order to disable the
 *   device
 */

export module device;

import std.string;

export namespace device {

enum class class_type {
    NONE,
    UART,
    CONSOLE,
    INTC,
    TIMER,
};

class device {
 public:
    constexpr static class_type dev_type = class_type::NONE;

    device(std::string const& name) : name_(name) {}

    std::string const& name() { return name_; }

    virtual class_type type() const { return device::dev_type; }
    virtual void init() {}
    virtual void deinit() {}
    virtual ~device() {}

    // delete copy and move constructors
    device(device const&) = delete;
    device(device&&) = delete;

 private:
    std::string name_;
};

}  // namespace device
