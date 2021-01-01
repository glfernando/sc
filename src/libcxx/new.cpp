/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

#include <errcodes.h>

import lib.heap;
import lib.exception;

#include <stddef.h>

using sc::lib::heap::alloc;
using sc::lib::heap::free;
using sc::lib::exception::exception;

void* operator new(size_t size)
{
    void* ptr = alloc(size);
    if (!ptr) throw exception("new failed", ERR_NO_MEMORY);
    return ptr;
}

void operator delete(void* p) noexcept { free(p); }
void* operator new[](size_t size) { return operator new(size); }
void operator delete[](void* p) noexcept { operator delete(p); }
void* operator new(size_t, void* p) { return p; }
