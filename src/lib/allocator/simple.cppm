/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

module;

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

export module lib.allocator.simple;

export namespace sc::lib::allocator {

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

    chunk* split_at(size_t offset)
    {
        if (offset <= CHUNK_SIZE)
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

    static chunk* from_mem_ptr(void* ptr)
    {
        return reinterpret_cast<chunk*>(reinterpret_cast<uintptr_t>(ptr) - CHUNK_SIZE);
    }
};

constexpr uint32_t MIN_ALIGNMENT = CHUNK_SIZE;
struct chunk;

class simple {
 public:
    simple(uint8_t* start, uint8_t* end) noexcept : start(start), end(end) {}
    void init() noexcept;
    void* alloc(size_t size, size_t align) noexcept;
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

            iterator& operator++()
            {
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

    chunk* find_free(size_t size, size_t align) noexcept;
    void dump() noexcept;

    uint8_t* const start;
    uint8_t* const end;
    chunk* free_chunk;
    chunks chunks;
};

}  // namespace sc::lib::allocator

// implementation

namespace {

// computes the offset that needs to be appiled to the pinter in order to make it aligned to @align
size_t align_offset(void* ptr, size_t align)
{
    uintptr_t ptr_val = reinterpret_cast<uintptr_t>(ptr);
    size_t mask = align - 1;

    return ((ptr_val + mask) & ~mask) - ptr_val;
}

}  // namespace

namespace sc::lib::allocator {

void simple::init() noexcept
{
    // TODO: assert valid parameters

    chunks.first = reinterpret_cast<chunk*>(start);
    chunks.last = reinterpret_cast<chunk*>(end);

    free_chunk = chunks.first;
    free_chunk->state = FREE;
    free_chunk->size = end - start - CHUNK_SIZE;
}

void* simple::alloc(size_t const size, size_t align) noexcept
{
    // only power of 2 alignment is allowed
    if (align & (align - 1))
        return nullptr;

    if (align < MIN_ALIGNMENT)
        align = MIN_ALIGNMENT;

    chunk* c = find_free(size, align);
    if (!c)
        return nullptr;

    size_t offset = align_offset(c->mem_ptr(), align);

    if (offset)
        c = c->split_at(offset - CHUNK_SIZE);

    auto tmp = c->size;

    c->state = USED;
    c->size = size;

    auto nc = c->next();
    nc->state = FREE;
    nc->size = tmp - size - CHUNK_SIZE;

    // update free chunk pointer
    free_chunk = nc;

    return c->mem_ptr();
}

void simple::free(void* p) noexcept
{
    if (!p)
        return;

    chunk* c = chunk::from_mem_ptr(p);
    if (c < chunks.first || c >= chunks.last) {
        printf("free: invalid pointer %p\n", c);
        return;
    }

    if (c->state == FREE) {
        printf("free: pointer already freed %p\n", c);
    } else if (c->state == USED) {
        c->state = FREE;
    } else {
        printf("free: invalid chunk state %x\n", c->state);
    }
}

chunk* simple::find_free(size_t size, size_t align) noexcept
{
    // check if cached free is enough
    auto c = free_chunk;
    auto aligned_size = align_offset(c->mem_ptr(), align) + size;

    if (c->state == FREE && c->size >= aligned_size)
        return c;

    // iterate over all chunks to fine one
    for (auto& chunk : chunks) {
        if (c->state != FREE)
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

void simple::dump() noexcept
{
    for (auto& c : chunks) {
        printf("chunk=%p state = %x, size %u\n", &c, c.state, c.size);
    }
}

}  // namespace sc::lib::allocator
