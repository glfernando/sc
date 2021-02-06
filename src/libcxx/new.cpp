/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

import lib.heap;

#include <stddef.h>

using sc::lib::heap::alloc;
using sc::lib::heap::free;

void* operator new(size_t size)
{
    // TODO: throw exception once they are available
    return alloc(size);
}

void operator delete(void* p) noexcept { free(p); }
void* operator new[](size_t size) { return operator new(size); }
void operator delete[](void* p) noexcept { operator delete(p); }
void* operator new(size_t, void* p) { return p; }
