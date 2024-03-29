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
import std.vector;
import lib.exception;

using lib::exception;
using std::vector;

export namespace device {

enum class class_type {
    NONE,
    UART,
    CONSOLE,
    INTC,
    TIMER,
    GPIO,
    PWM,
    I2C,
    SPI,
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

namespace device {
static vector<device*> devices;
}

export namespace device {

class manager {
 public:
    static void register_device(device* dev) { devices.push_back(dev); }

    static void unregister_device(device* dev) {
        for (auto it = devices.begin(); it != devices.end(); ++it) {
            if (*it == dev) {
                devices.erase(it);
                return;
            }
        }
    }

    static device* find(class_type type) {
        for (auto dev : devices) {
            if (dev->type() == type)
                return dev;
        }
        return nullptr;
    }

    // TODO: create Device concept
    template <typename D>
    static D* find() {
        for (auto dev : devices) {
            if (dev->type() == D::dev_type)
                return static_cast<D*>(dev);
        }
        return nullptr;
    }

    static vector<device*> find_all(class_type type) {
        vector<device*> dev_list;

        for (auto dev : devices) {
            if (dev->type() == type)
                dev_list.push_back(dev);
        }
        return dev_list;
    }

    // TODO: create Device concept
    template <typename D>
    static vector<D*> find_all() {
        vector<D*> dev_list;
        for (auto dev : devices) {
            if (dev->type() == D::dev_type)
                dev_list.push_back(static_cast<D*>(dev));
        }
        return dev_list;
    }

    static device* find(class_type type, std::string const& name) {
        for (auto dev : devices) {
            if (dev->type() == type && dev->name() == name)
                return dev;
        }
        return nullptr;
    }

    // TODO: create Device concept
    template <typename D>
    static D* find(std::string const& name) {
        for (auto dev : devices) {
            if (dev->type() == D::dev_type && dev->name() == name)
                return static_cast<D*>(dev);
        }
        return nullptr;
    }
};

}  // namespace device
