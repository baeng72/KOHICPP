#include "kmemory.hpp"

#include "core/logger.hpp"
#include "platform/platform.hpp"

#include <cstring>
#include <cstdio>

ccharp memory_system::memory_tag_strings[MEMORY_TAG_MAX_TAGS]={
    "UNKNOWN    ",
    "ARRAY      ",
    "DARRAY     ",
    "DICT       ",
    "RING_QUEUE ",
    "BST        ",
    "STRING     ",
    "APPLICATION",
    "JOB        ",
    "TEXTURE    ",
    "MAT_INST   ",
    "RENDERER   ",
    "GAME       ",
    "TRANSFORM  ",
    "ENTITY     ",
    "ENTITY_NODE",
    "SCENE      "
};



memory_system*memory_system::instance(){
    static memory_system mem_system;
    return &mem_system;
}


void memory_system::initialize(){
    total_allocated=0;
    platform_zero_memory(tagged_allocations,sizeof(tagged_allocations));
}

void memory_system::shutdown(){

}

void* memory_system::allocate(u64 size, memory_tag tag){
    if(tag == MEMORY_TAG_UNKNOWN){
        KWARN("kallocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    total_allocated += size;
    tagged_allocations[tag] += size;

    //TODO: memory alignment
    void * block = platform_allocate(size,false);
    platform_zero_memory(block, size);
    return block;
}

void memory_system::free(void* block, u64 size, memory_tag tag){
    if(tag == MEMORY_TAG_UNKNOWN){
        KWARN("kfree called using MEMORY_TAG_UNKNOWN. Re class this allocation.");
    }

    total_allocated -= size;
    tagged_allocations[tag] -= size;

    //TODO: memory alignment
    platform_free(block,false);
}


inline void * memory_system::zero_memory(void * block, u64 size){
    return platform_zero_memory(block, size);
}

inline void * memory_system::copy_memory(void * dest,const void * src, u64 size){
    return platform_copy_memory(dest, src, size);
}

inline void * memory_system::set_memory(void * dest, i32 value, u64 size){
    return platform_set_memory(dest, value, size);
}


char * memory_system::getMemoryUsageStr(void){
    constexpr u64 kib = 1024;
    constexpr u64 mib = kib * 1024;
    constexpr u64 gib = mib * 1024;
    
    char buffer[8192] = "System memory use (tagged):\n";
    u64 offset = strlen(buffer);
    for(u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++ i){
        char unit[4] = "XiB";
        u64 allocated = tagged_allocations[i];
        float amount = 1.f;
        if(allocated >= gib)
        {
            unit[0] = 'G';
            amount = allocated / (float)gib;
        }
        else if(allocated >= mib)
        {
            unit[0] = 'M';
            amount = allocated / (float)mib;
        }
        else if(allocated >= kib)
        {
            unit[0] = 'K';
            amount = allocated / (float)mib;
        }else
        {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (float)allocated;
        }

        i32 length = snprintf(buffer+offset,8192, "  %s: %.2f%s\n",memory_tag_strings[i],amount, unit);
        offset += length;
    }
    char * out_string = _strdup(buffer);
    return out_string;
}