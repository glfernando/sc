/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stddef.h>
#include <stdint.h>

extern "C" uint8_t __heap_start[];
extern "C" uint8_t __heap_end[];

export module lib.heap;

import lib.allocator.simple;

static lib::allocator::simple heap(__heap_start, __heap_end);

export namespace lib::heap {

void init() {
    ::heap.init();
}

void* alloc(size_t size, size_t align = 8) {
    return ::heap.alloc(size, align);
}

void* realloc(void* p, size_t size, size_t align = 8) {
    return ::heap.realloc(p, size, align);
}

void free(void* p) {
    return ::heap.free(p);
}

}  // namespace lib::heap
