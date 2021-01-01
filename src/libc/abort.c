/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

#include <stdio.h>

void abort(void)
{
    printf("abort\n");
    for (;;) {}
}
