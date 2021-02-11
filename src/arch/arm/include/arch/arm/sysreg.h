/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

#pragma once

#define sysreg_read(reg)                                 \
    ({                                                   \
        unsigned long v;                                 \
        __asm__ __volatile__("mrs %0, " #reg : "=r"(v)); \
        v;                                               \
    })

#define sysreg_write(reg, v) ({ __asm__ __volatile__("msr " #reg ", %0" ::"r"(v)); })
