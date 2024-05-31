#pragma once

#include "defines.hpp"

#include <vulkan/vulkan.h>

struct vulkan_context{
    VkInstance instance{VK_NULL_HANDLE};
    VkAllocationCallbacks* allocator{nullptr};
};