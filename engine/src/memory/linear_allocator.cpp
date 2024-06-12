#include "linear_allocator.hpp"

#include "core/kmemory.hpp"
#include "core/logger.hpp"

void linear_allocator::create(u64 total_size_, void* memory_){
    total_size = total_size_;
    allocated = 0;
    owns_memory = memory_ == nullptr;
    if(memory_){
        memory = memory_;
    }else{
        memory = kallocate(total_size_, MEMORY_TAG_LINEAR_ALLOCATOR);
    }
}

void linear_allocator::destroy(){
    allocated = 0;
    if(owns_memory && memory){
        kfree(memory, total_size, MEMORY_TAG_LINEAR_ALLOCATOR);
    }
    memory = 0;
    total_size = 0;
    owns_memory = false;
}

void * linear_allocator::allocate(u64 size){
    if(memory){
        if(allocated + size > total_size){
            u64 remaining = total_size - allocated;
            KERROR("%s - Tried to allocate %lluB, only %lluB remaining.", __FUNCTION__, size, remaining);
            return nullptr;
        }

        void * block = ((u8*)memory) + allocated;
        allocated += size;
        return block;
    }

    KERROR("%s - provided allocator not initialized.", __FUNCTION__);
    return nullptr;
}

void linear_allocator::free_all(){
    if(memory){
        allocated = 0;
        kzero_memory(memory,total_size);
    }
}