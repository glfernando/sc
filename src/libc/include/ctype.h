/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (C) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int isprint(int c) {
    return c >= 0x20 && c < 0x7f;
}

#ifdef __cplusplus
}
#endif
