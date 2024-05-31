#pragma once

#include "defines.hpp"
#include "core/asserts.hpp"

#include <vulkan/vulkan.h>

//Checks the given expression's return value against VK_SUCCESS.
#define VK_CHECK(expr) \
    {\
        KASSERT(expr == VK_SUCCESS);\
    }

struct vulkan_context{
    VkInstance instance{VK_NULL_HANDLE};
    VkAllocationCallbacks* allocator{nullptr};
#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debug_messenger;    
    PFN_vkSetDebugUtilsObjectNameEXT pfnSetObjectName;
    PFN_vkCmdBeginDebugUtilsLabelEXT pfnBeginLabel;
    PFN_vkCmdEndDebugUtilsLabelEXT   pfnEndLabel;    
#endif        
};