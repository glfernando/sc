/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

#pragma once

#include <stddef.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#ifdef __cplusplus
extern "C" {
#endif

void* malloc(size_t size);
void* calloc(size_t nmemb, size_t size);
void* realloc(void* ptr, size_t size);
void free(void*);

void abort(void);
void qsort(void* aa, size_t n, size_t es, int (*cmp)(const void*, const void*));

#ifdef __cplusplus
}
#endif
