#pragma once

#include "defines.hpp"
#include "containers/darray.hpp"

struct vulkan_context;

class KAPI platform_system{
    void* internal_state;
    static platform_system*state_ptr;
    public:
    bool startup(ccharp application_name,i32 x, i32 y, i32 width, i32 height);
    void shutdown();
    static void get_required_extensions_names(darray<ccharp>&names_array);
    static bool create_vulkan_surface(vulkan_context*context);
    static bool pump_messages();
};




void * platform_allocate(u64 size, bool aligned);
void platform_free(void*block, bool aligned);
void* platform_zero_memory(void* block, u64 size);
void* platform_copy_memory(void*dest, const void* source, u64 size);
void *platform_set_memory(void*dest, i32 value, u64 size);

void platform_console_write(ccharp message, u8 color);
void platform_console_write_error(ccharp message, u8 color);

f64 platform_get_absolute_time();

void platform_sleep(u64 ms);



