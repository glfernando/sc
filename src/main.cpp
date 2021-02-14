/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

#include <app/shell.h>

// allow to override main
__attribute__((weak)) int main() {
    // just run the shell, there is nothing else to do
    shell_run();
    return 0;
}
