/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (C) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

#pragma once

#include <stdarg.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int vsprintf(char* buf, const char* fmt, va_list va);
int vsnprintf(char* buf, size_t size, const char* fmt, va_list va);
int sprintf(char* buf, const char* fmt, ...) __attribute__((__format__(__printf__, 2, 3)));
int snprintf(char* buf, size_t size, const char* fmt, ...)
    __attribute__((__format__(__printf__, 3, 4)));
int printf(const char* fmt, ...) __attribute__((__format__(__printf__, 1, 2)));

/*
 * Allow users to provide their own putchar method. Mostly will be used once a
 * console driver us running
 *
 * @func    Custom putchar function pointer
 *
 */

void printf_set_putchar_func(int (*func)(int c));

#ifdef __cplusplus
}
#endif
