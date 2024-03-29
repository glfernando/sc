/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

export module lib.allocator.simple;
import lib.lock;
import arch.aarch64.mte;

export namespace lib::allocator {

enum chunk_state : uint32_t {
    FREE = 0xA110F7EE,
    USED = 0xA10045ED,
};

#define CHUNK_SIZE sizeof(chunk)

struct chunk {
    chunk_state state;
    uint32_t size;

    uint8_t* to_ptr() { return reinterpret_cast<uint8_t*>(this); }

    uint8_t const* to_ptr() const { return reinterpret_cast<uint8_t const*>(this); }

    uint8_t* mem_ptr() { return reinterpret_cast<uint8_t*>(this) + CHUNK_SIZE; }

    chunk* next() { return reinterpret_cast<chunk*>(mem_ptr() + size); }

    chunk* split_at(size_t offset) {
        if (size <= CHUNK_SIZE)
            return nullptr;

        if (offset > size - CHUNK_SIZE)
            return nullptr;
        size_t total_size = size;
        size = offset;
        chunk* c = next();
        c->size = total_size - offset - CHUNK_SIZE;
        return c;
    }

    static chunk* from_ptr(void* ptr) { return reinterpret_cast<chunk*>(ptr); }

    static chunk* from_mem_ptr(void* ptr) {
        return reinterpret_cast<chunk*>(reinterpret_cast<uintptr_t>(ptr) - CHUNK_SIZE);
    }
};

constexpr uint32_t MIN_ALIGNMENT = 16;
struct chunk;

class simple {
 public:
    constexpr simple(uint8_t* start, uint8_t* end) noexcept : start(start), end(end) {}
    void init() noexcept;
    void* alloc(size_t size, size_t align) noexcept;
    void* realloc(void* p, size_t size, size_t align) noexcept;
    void free(void* p) noexcept;

 private:
    struct chunks {
        chunk* first;
        chunk* last;

        struct iterator {
            chunk* ptr;
            chunk* last;

            bool operator==(const iterator& other) const { return ptr == other.ptr; }
            bool operator!=(const iterator& other) const { return !(*this == other); }

            chunk& operator*() const { return *ptr; }

            iterator& operator++() {
                ptr = ptr->next();
                // sanity check
                if (ptr > last) {
                    printf("alloc: chunk beyond limits %p\n", ptr);
                    ptr = last;
                } else if (ptr < last && ptr->state != FREE && ptr->state != USED) {
                    printf("alloc: invalid chunk state %x\n", ptr->state);
                    ptr = last;
                }
                return *this;
            }
        };

        iterator begin() const { return iterator{first, last}; }

        iterator end() const { return iterator{last, last}; }
    };

    void* alloc_notag(size_t size, size_t align) noexcept;

    chunk* find_free(size_t size, size_t align) noexcept;
    void dump() noexcept;

    uint8_t* const start;
    uint8_t* const end;
    chunk* free_chunk;
    chunks chunks;
    lock lock;
};

}  // namespace lib::allocator

// implementation

namespace {

template <typename T>
constexpr T align_up(T v, size_t a) {
    size_t mask = a - 1;
    return (v + mask) & ~mask;
}

// computes the offset that needs to be appiled to the pinter in order to make it aligned to @align
size_t align_offset(void* ptr, size_t align) {
    uintptr_t ptr_val = reinterpret_cast<uintptr_t>(ptr);
    size_t mask = align - 1;

    return ((ptr_val + mask) & ~mask) - ptr_val;
}

}  // namespace

namespace lib::allocator {

void simple::init() noexcept {
    // TODO: assert valid parameters

    chunks.first = reinterpret_cast<chunk*>(start);
    chunks.last = reinterpret_cast<chunk*>(end);

    free_chunk = chunks.first;
    free_chunk->state = FREE;
    free_chunk->size = end - start - CHUNK_SIZE;
}

void* simple::alloc_notag(size_t size, size_t align) noexcept {
    // only power of 2 alignment is allowed
    if (align & (align - 1))
        return nullptr;

    if (align < MIN_ALIGNMENT)
        align = MIN_ALIGNMENT;

    // MTE requires minimum granurality of 16 bytes
    size = align_up(size, 16);

    slock_irqsafe guard{lock};
    chunk* c = find_free(size, align);
    if (!c)
        return nullptr;

    size_t offset = align_offset(c->mem_ptr(), align);

    if (offset)
        c = c->split_at(offset - CHUNK_SIZE);

    auto actual_size = align_up(size + CHUNK_SIZE, MIN_ALIGNMENT) - CHUNK_SIZE;
    actual_size = actual_size > c->size ? c->size : actual_size;

    auto nc = c->split_at(actual_size);
    if (nc) {
        nc->state = FREE;
        // update free chunk pointer
        free_chunk = nc;
    }

    c->state = USED;
    return c->mem_ptr();
}

static void* tag_region(void* ptr, size_t size, bool random = false) {
    uintptr_t addr = (uintptr_t)ptr;

    if (random) {
        addr = aarch64::irg(addr);
        ptr = (void *)addr;
    }

    for (size_t i = 0; i < size; i += 16, addr += 16) {
        aarch64::stg(addr);
    }

    return ptr;
}

void* simple::alloc(size_t size, size_t align) noexcept {
    auto ptr = simple::alloc_notag(size, align);
    if (ptr) {
        ptr = tag_region(ptr, size, true);
    }
    return ptr;
}

static void* untag_addr(void *p) {
    uintptr_t addr = (uintptr_t)p;
    addr &=  ~(0xfUL << 56);
    return (void*)addr;
}

void simple::free(void* p) noexcept {
    if (!p)
        return;

    p = untag_addr(p);

    chunk* c = chunk::from_mem_ptr(p);
    if (c < chunks.first || c >= chunks.last) {
        printf("free: invalid pointer %p\n", c);
        return;
    }

    // tag region with untag address
    tag_region(p, c->size);

    if (c->state == FREE) {
        printf("free: pointer already freed %p\n", c);
    } else if (c->state == USED) {
        c->state = FREE;
    } else {
        printf("free: invalid chunk state %x\n", c->state);
    }
}

void* simple::realloc(void* p, size_t size, size_t align) noexcept {
    if (!p)
        return alloc(size, align);

    chunk* c = chunk::from_mem_ptr(p);
    if (c < chunks.first || c >= chunks.last) {
        printf("free: invalid pointer %p\n", c);
        return nullptr;
    }

    // simple copy
    void* ptr = alloc(size, align);
    if (!ptr)
        return nullptr;

    memcpy(ptr, p, c->size);
    free(p);

    return ptr;
}

chunk* simple::find_free(size_t size, size_t align) noexcept {
    // check if cached free is enough
    auto c = free_chunk;
    auto aligned_size = align_offset(c->mem_ptr(), align) + size;

    if (c->state == FREE && c->size >= aligned_size)
        return c;

    // iterate over all chunks to fine one
    for (auto& chunk : chunks) {
        if (chunk.state != FREE)
            continue;

        // merge free chunks
        for (auto tmp = chunk.next(); tmp < chunks.last && tmp->state == FREE; tmp = tmp->next())
            chunk.size += tmp->size + CHUNK_SIZE;

        auto aligned_size = align_offset(chunk.mem_ptr(), align) + size;
        if (chunk.size >= aligned_size)
            return &chunk;
    }

    return nullptr;
}

void simple::dump() noexcept {
    for (auto& c : chunks) {
        printf("chunk=%p state = %x, size %u\n", &c, c.state, c.size);
    }
}

}  // namespace lib::allocator
