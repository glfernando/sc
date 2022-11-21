/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

#include <errcodes.h>
#include <stddef.h>

import lib.heap;
import lib.exception;

using lib::exception;
using lib::heap::alloc;
using lib::heap::free;

namespace std {

enum class align_val_t : size_t {};

}

void* operator new(size_t size) {
    void* ptr = alloc(size, __STDCPP_DEFAULT_NEW_ALIGNMENT__);
    if (!ptr)
        throw exception("new failed", ERR_NO_MEMORY);
    return ptr;
}

void operator delete(void* p) noexcept {
    free(p);
}

void* operator new[](size_t size) {
    return operator new(size);
}

void operator delete[](void* p) noexcept {
    operator delete(p);
}

void* operator new(size_t, void* p) {
    return p;
}

void* operator new[](size_t, void* p) {
    return p;
}

void* operator new(size_t size, std::align_val_t align) {
    void* ptr = alloc(size, static_cast<size_t>(align));
    if (!ptr)
        throw exception("new failed", ERR_NO_MEMORY);
    return ptr;
}

void* operator new[](size_t size, std::align_val_t align) {
    return operator new(size, align);
}

void operator delete(void* p, std::align_val_t) noexcept {
    free(p);
}

void operator delete[](void* p, std::align_val_t) noexcept {
    free(p);
}
