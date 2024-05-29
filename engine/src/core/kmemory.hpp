#pragma once

#include "defines.hpp"

enum memory_tag{
     MEMORY_TAG_UNKNOWN,
    MEMORY_TAG_ARRAY,
    MEMORY_TAG_DARRAY,
    MEMORY_TAG_DICT,
    MEMORY_TAG_RING_QUEUE,
    MEMORY_TAG_BST,
    MEMORY_TAG_STRING,
    MEMORY_TAG_APPLICATION,
    MEMORY_TAG_JOB,
    MEMORY_TAG_TEXTURE,
    MEMORY_TAG_MATERIAL_INSTANCE,
    MEMORY_TAG_RENDERER,
    MEMORY_TAG_GAME,
    MEMORY_TAG_TRANSFORM,
    MEMORY_TAG_ENTITY,
    MEMORY_TAG_ENTITY_NODE,
    MEMORY_TAG_SCENE,

    MEMORY_TAG_MAX_TAGS
};

class KAPI memory_system{
    static ccharp memory_tag_strings[];
    u64 total_allocated{0};
    u64 tagged_allocations[MEMORY_TAG_MAX_TAGS];
    memory_system(){}
    char* getMemoryUsageStr();
    public:
    memory_system(memory_system const&)=delete;
    void operator=(memory_system const&)=delete;
    char* get_memory_usage_str() { return getMemoryUsageStr(); }//compiler complains when I implement get_memory_usage_str(), so call another?
    static memory_system*instance();
    void initialize();
    void shutdown();
    void *allocate(u64 size, memory_tag tag);
    void free(void*block, u64 size, memory_tag tag);
   
    static void* zero_memory(void*block,u64 size);
    static void * copy_memory(void*dest, const void*source, u64 size);
    static void* set_memory(void*dest, i32 value, u64 size);

};

#define kallocate(size, tag) (memory_system::instance()->allocate((size),(tag)))
#define kfree(block, size, tag) (memory_system::instance()->free((block),(size),(tag)))
#define kzero_memory(block, size) (memory_system::zero_memory((block),(size)))
#define kcopy_memory(dest,source,size) (memory_system::copy_memory((dest),(source),(size)))
#define kset_memory(dest, value, size) (memory_system::set_memory((dest), (value), (size)))
#define get_memory_usage_str() (memory_system::instance()->get_memory_usage_str())