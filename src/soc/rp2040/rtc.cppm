/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <app/shell.h>
#include <stdint.h>
#include <string.h>

export module soc.rp2040.rtc;

import lib.reg;
import soc.rp2040.address_map;
import soc.rp2040.reset;
import lib.fmt;
import std.string;

using lib::reg::reg32;
using namespace soc::rp2040::address_map;
using lib::fmt::println;
using lib::fmt::sprint;

namespace {

constexpr unsigned RTC_CLK_FREQ = 46875;

// clang-format off
enum reg_offset : uint32_t {
    CLKDIV_M1   = 0x00,
    SETUP_0     = 0x04,
    SETUP_1     = 0x08,
    CTRL        = 0x0c,
    IRQ_SETUP_0 = 0x10,
    IRQ_SETUP_1 = 0x14,
    RTC_1       = 0x18,
    RTC_0       = 0x1c,

};

enum CTRL_bits : uint32_t {
    ENABLE  = 1 << 0,
    ACTIVE  = 1 << 1,
    LOAD    = 1 << 4,
};
// clang-format on

struct rtc_datetime {
    union {
        struct {
            uint32_t sec : 6;
            uint32_t : 2;
            uint32_t min : 6;
            uint32_t : 2;
            uint32_t hour : 5;
            uint32_t : 3;
            uint32_t dotw : 3;
            uint32_t : 5;
        };
        uint32_t data0;
    };
    union {
        struct {
            uint32_t day : 5;
            uint32_t : 3;
            uint32_t month : 4;
            uint32_t year : 12;
            uint32_t : 8;
        };
        uint32_t data1;
    };
};

static_assert(sizeof(rtc_datetime) == 8);

volatile uint32_t& reg(uint32_t offset) {
    return reg32(RTC_BASE + offset);
}

const char* dotw_to_str(uint8_t dotw) {
    switch (dotw) {
    case 0:
        return "Sun";
    case 1:
        return "Mon";
    case 2:
        return "Tue";
    case 3:
        return "Wed";
    case 4:
        return "Thu";
    case 5:
        return "Fri";
    case 6:
        return "Sat";
    }
    return "Unknown";
}

}  // namespace

export namespace soc::rp2040::rtc {

struct datetime_t {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t dotw;
    uint8_t day;
    uint8_t month;
    uint16_t year;

    operator std::string() {
        return sprint("{:04}/{:02}/{:02} {} {:02}:{:02}:{:02}", year, month, day, dotw_to_str(dotw),
                      hour, min, sec);
    }
};

void init() {
    // reset RTC module
    reset::set(reset::RTC);
    reset::clear(reset::RTC, true);

    reg(CLKDIV_M1) = RTC_CLK_FREQ - 1;

    reg(CTRL) |= ENABLE;
}

datetime_t datetime() {
    rtc_datetime val;
    val.data0 = reg(RTC_0);
    val.data1 = reg(RTC_1);
    return datetime_t{
        .sec = static_cast<uint8_t>(val.sec),
        .min = static_cast<uint8_t>(val.min),
        .hour = static_cast<uint8_t>(val.hour),
        .dotw = static_cast<uint8_t>(val.dotw),
        .day = static_cast<uint8_t>(val.day),
        .month = static_cast<uint8_t>(val.month),
        .year = static_cast<uint16_t>(val.year),
    };
}

void datetime(datetime_t time) {
    rtc_datetime val = {
        .sec = static_cast<uint8_t>(time.sec),
        .min = static_cast<uint8_t>(time.min),
        .hour = static_cast<uint8_t>(time.hour),
        .dotw = static_cast<uint8_t>(time.dotw),
        .day = static_cast<uint8_t>(time.day),
        .month = static_cast<uint8_t>(time.month),
        .year = static_cast<uint16_t>(time.year),
    };

    // disable RTC
    reg(CTRL) = 0;

    while (reg(CTRL) & ACTIVE) {}

    reg(SETUP_1) = val.data0;
    reg(SETUP_0) = val.data1;

    reg(CTRL) |= LOAD;
    reg(CTRL) |= ENABLE;

    while (~reg(CTRL) & ACTIVE) {}
}

}  // namespace soc::rp2040::rtc

using namespace soc::rp2040::rtc;

void cmd_date_usage() {
    println("date [yy] [mm] [day] [hh] [mm] [ss] [dotw]");
}

static int cmd_date(int argc, char const* argv[]) {
    if (argc == 1) {
        println("{}", soc::rp2040::rtc::datetime());
        return 0;
    }

    if (argc > 8) {
        cmd_date_usage();
        return 0;
    }

    auto val = datetime();

    if (argc >= 2) {
        uint16_t year = strtoul(argv[1], nullptr, 0);
        // TODO: add validation?
        val.year = year;
    }

    if (argc >= 3) {
        auto month = strtoul(argv[2], nullptr, 0);
        if (month == 0 || month > 12) {
            println("invalid month value {}", month);
        }
        val.month = month;
    }

    if (argc >= 4) {
        auto day = strtoul(argv[3], nullptr, 0);
        if (day == 0 || day > 31) {
            println("invalid day value {}", day);
        }
        val.day = day;
    }

    if (argc >= 5) {
        auto hour = strtoul(argv[4], nullptr, 0);
        if (hour > 23) {
            println("invalid hour value {}", hour);
        }
        val.hour = hour;
    }

    if (argc >= 6) {
        auto min = strtoul(argv[5], nullptr, 0);
        if (min > 59) {
            println("invalid hour value {}", min);
        }
        val.min = min;
    }

    if (argc >= 7) {
        auto sec = strtoul(argv[6], nullptr, 0);
        if (sec > 59) {
            println("invalid sec value {}", sec);
        }
        val.sec = sec;
    }

    if (argc >= 8) {
        auto dotw = strtoul(argv[7], nullptr, 0);
        if (dotw > 6) {
            println("invalid day of the week value {}", dotw);
        }
        val.dotw = dotw;
    }

    datetime(val);

    return 0;
}

shell_declare_static_cmd(date, "read/set daytime", cmd_date, cmd_date_usage);
