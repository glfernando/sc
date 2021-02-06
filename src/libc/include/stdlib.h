/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void* malloc(size_t size);
void* calloc(size_t nmemb, size_t size);
void free(void*);

#ifdef __cplusplus
}
#endif
