/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module device.gpio;

export import device;
import std.string;

export namespace device {

class gpio : public device {
 public:
    constexpr static class_type dev_type = class_type::GPIO;
    class_type type() const override final { return gpio::dev_type; }

    gpio(std::string const& name) : device(name) {}

    enum class dir {
        OUTPUT,
        INPUT,
    };

    enum class trigger {
        NONE,
        HIGH,
        LOW,
        RISING,
        FALLING,
        BOTH,
    };

    enum class pull {
        NONE,
        DOWN,
        UP,
    };

    struct config_t {
        dir dir;
        trigger trigger;
        pull pull;
    };

    // gpio interface
    virtual void config(unsigned gpio, config_t const& config) = 0;
    virtual void set(unsigned gpio, bool val) = 0;
    virtual bool get(unsigned gpip) = 0;
};

// goio device concept
// This can be use to create generic code that does not use vtable

template <typename T>
concept Gpio = requires(T t, bool val, unsigned gpio, gpio::config_t config) {
    t.config(gpio, config);
    val = t.get(gpio);
    t.set(gpio);
};

};  // namespace device
