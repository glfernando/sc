
/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

export module core.cpu.armv8.common;

#include <arch/aarch64/sysreg.h>

export namespace core::cpu {

unsigned cpu_id() {
    unsigned mpidr = sysreg_read(mpidr_el1);
    unsigned id = mpidr & 0xffffff;

    if (id >= 0x100) {
        unsigned cluster = (id >> 8) & 0xff;
        id = (id & 0xff) + CONFIG_CPU_AFF0_CPU_MAX * cluster;
    }

    return id;
}

}  // namespace core::cpu
