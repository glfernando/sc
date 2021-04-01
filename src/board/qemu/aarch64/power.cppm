/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module board.power;

enum SH_OP {
    SH_SYS_EXIT = 0x18,
};

enum SH_EXCEP_TYPE {
    ADP_Stopped_ApplicationExit = 0x20026,
};

struct sh_sys_exit_params {
    long excep_type;
    long subcode;
};

static __attribute__((naked)) long semihost(unsigned long, unsigned long) {
    asm volatile(
        "hlt 0xf000\n"
        "ret");
}

export namespace board {

[[noreturn]] void reboot(int) {
    // TODO: implement
    for (;;)
        asm volatile("wfi");
}

[[noreturn]] void poweroff(int exit_code = 0) {
    sh_sys_exit_params params = {ADP_Stopped_ApplicationExit, exit_code};
    semihost(SH_SYS_EXIT, reinterpret_cast<unsigned long>(&params));

    for (;;)
        asm volatile("wfi");
}

}  // namespace board
