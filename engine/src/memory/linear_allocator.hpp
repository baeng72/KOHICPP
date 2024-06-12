#pragma once

#include "defines.hpp"
#include <new>
struct KAPI linear_allocator{
    u64 total_size;
    u64 allocated;
    void * memory;
    bool owns_memory;
    void create(u64 total_size, void* memory);
    void destroy();

    void* allocate(u64 size);
    void free_all();
};
