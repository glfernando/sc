/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

#include <stdlib.h>
#include <string.h>

import lib.heap;

extern "C" void* malloc(size_t size) {
    return lib::heap::alloc(size);
}

extern "C" void* calloc(size_t nmemb, size_t size) {
    void* ptr = malloc(nmemb * size);
    if (ptr)
        memset(ptr, 0, nmemb * size);
    return ptr;
}

extern "C" void free(void* p) {
    return lib::heap::free(p);
}
