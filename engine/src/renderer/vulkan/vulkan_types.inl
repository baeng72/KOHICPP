#pragma once

#include "defines.hpp"
#include "core/asserts.hpp"

#include <vulkan/vulkan.h>

//Checks the given expression's return value against VK_SUCCESS.
#define VK_CHECK(expr) \
    {\
        KASSERT(expr == VK_SUCCESS);\
    }

struct vulkan_swapchain_support_info{
    VkSurfaceCapabilitiesKHR capabilities;
    u32 format_count;
    VkSurfaceFormatKHR *formats;
    u32 present_mode_count;
    VkPresentModeKHR * present_modes;
};
struct vulkan_physical_device_queue_family_info;
struct vulkan_physical_device_requirements;
struct vulkan_device{
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    vulkan_swapchain_support_info swapchain_support;
    i32 graphics_queue_index;
    i32 present_queue_index;
    i32 transfer_queue_index;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;
    operator VkPhysicalDevice(){return physical_device;}
    operator VkDevice(){return logical_device;}
    bool create(VkInstance instance, VkSurfaceKHR surface);
    void destroy(VkInstance instance);
    bool select_physical_device(VkInstance instance, VkSurfaceKHR surface);
    bool physical_device_meets_requirements(VkPhysicalDevice device,
        VkSurfaceKHR surface,
        const VkPhysicalDeviceProperties* curr_props,
        const VkPhysicalDeviceFeatures* curr_features,
        const vulkan_physical_device_requirements * requirements,
        vulkan_physical_device_queue_family_info * out_queue_info,
        vulkan_swapchain_support_info*out_swapchain_support);
    void query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface, vulkan_swapchain_support_info*out_support_info);
};

struct vulkan_context{
    VkInstance instance{VK_NULL_HANDLE};
    VkAllocationCallbacks* allocator{nullptr};
    VkSurfaceKHR surface{VK_NULL_HANDLE};
    
#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debug_messenger;    
    PFN_vkSetDebugUtilsObjectNameEXT pfnSetObjectName;
    PFN_vkCmdBeginDebugUtilsLabelEXT pfnBeginLabel;
    PFN_vkCmdEndDebugUtilsLabelEXT   pfnEndLabel;    
#endif        
    vulkan_device device;
};