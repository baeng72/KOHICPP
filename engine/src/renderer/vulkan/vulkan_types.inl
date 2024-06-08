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
struct vulkan_context;
struct vulkan_device{
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    vulkan_swapchain_support_info swapchain_support;
    i32 graphics_queue_index;
    i32 present_queue_index;
    i32 transfer_queue_index;

    VkQueue graphics_queue{VK_NULL_HANDLE};
    VkQueue present_queue{VK_NULL_HANDLE};
    VkQueue transfer_queue{VK_NULL_HANDLE};

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;

    VkFormat depth_format{VK_FORMAT_UNDEFINED};
#if defined(_DEBUG)
    PFN_vkSetDebugUtilsObjectNameEXT pfnSetObjectName;
    PFN_vkCmdBeginDebugUtilsLabelEXT pfnBeginLabel;
    PFN_vkCmdEndDebugUtilsLabelEXT   pfnEndLabel;  
#endif    

    operator VkPhysicalDevice(){return physical_device;}
    operator VkDevice(){return logical_device;}
    bool create(vulkan_context*context);
    void destroy(vulkan_context*context);
    bool select_physical_device(VkInstance instance, VkSurfaceKHR surface);
    bool physical_device_meets_requirements(VkPhysicalDevice device,
        VkSurfaceKHR surface,
        const VkPhysicalDeviceProperties* curr_props,
        const VkPhysicalDeviceFeatures* curr_features,
        const vulkan_physical_device_requirements * requirements,
        vulkan_physical_device_queue_family_info * out_queue_info,
        vulkan_swapchain_support_info*out_swapchain_support);
    void query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface, vulkan_swapchain_support_info*out_support_info);
#if defined(_DEBUG)
    void SetResourceName(vulkan_context*context, VkObjectType type, u64 handle, ccharp name);
#endif 
    bool detect_depth_format();
};

struct vulkan_image{
    VkImage handle{VK_NULL_HANDLE};
    VkDeviceMemory memory{VK_NULL_HANDLE};
    VkImageView view{VK_NULL_HANDLE};
    u32 width{0};
    u32 height{0};
    void create(vulkan_context*context,VkImageType image_type, u32 width, u32 height, 
        VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
        VkMemoryPropertyFlags memory_flags, 
        bool create_view,
        VkImageAspectFlags view_aspect_flags);
    void create_image_view(vulkan_context*context, VkFormat format, VkImageAspectFlags aspect_flags);
    void destroy(vulkan_context*context);
};


enum vulkan_render_pass_state{
    READY,
    RECORDING,
    IN_RENDER_PASS,
    RECORDING_ENDED,
    SUBMITTED,
    NOT_ALLOCATED
};

struct vulkan_command_buffer;
struct vulkan_renderpass{
    VkRenderPass handle{VK_NULL_HANDLE};
    f32 x{0.f};
    f32 y{0.f};
    f32 w{0.f};
    f32 h{0.f};
    f32 r{0.f};
    f32 g{0.f};
    f32 b{0.f};
    f32 a{0.f};
    f32 depth{0.f};
    u32 stencil{0};
    vulkan_render_pass_state state;
    void create(vulkan_context* context, f32 x, f32 y, f32 w, f32 h, f32 r, f32 g, f32 b, f32 a, f32 depth, u32 stencil);
    void destroy(vulkan_context* context);
    void begin(vulkan_command_buffer* command_buffer, VkFramebuffer frame_buffer);
    void end(vulkan_command_buffer* command_buffer);
};

struct vulkan_swapchain{
    
    VkSurfaceFormatKHR image_format{VK_FORMAT_UNDEFINED};
    u32 max_frames_in_flight{0};
    VkSwapchainKHR handle{VK_NULL_HANDLE};
    u32 image_count{0};
    VkImage * images{nullptr};
    VkImageView * views{nullptr};

    vulkan_image depth_attachment;
    void create(vulkan_context*context, u32 width, u32 height);
    void recreate(vulkan_context*context, u32 width, u32 height);
    void destroy(vulkan_context*context);
    bool acquire_next_image_index(vulkan_context*context, u64 timeout_ns, VkSemaphore image_available_semaphore, VkFence fence, u32*out_image_index);
    void present(vulkan_context*context,VkQueue graphics_queue, VkQueue present_queue, VkSemaphore render_complete_semaphore, u32 present_image_index);
    
};

enum vulkan_command_buffer_state{
    COMMAND_BUFFER_STATE_READY,
    COMMAND_BUFFER_STATE_RECORDING,
    COMMAND_BUFFER_STATE_IN_RENDER_PASS,
    COMMAND_BUFFER_STATE_RECORDING_ENDED,
    COMMAND_BUFFER_STATE_SUBMITTED,
    COMMAND_BUFFER_STATE_NO_ALLOCATED
};

struct vulkan_command_buffer{
    VkCommandBuffer handle{VK_NULL_HANDLE};
    //Command buffer state
    vulkan_command_buffer_state state;
};


struct vulkan_context{
    // The framebuffer's current width, height.
    u32 framebuffer_width{0};
    u32 framebuffer_height{0};
    VkInstance instance{VK_NULL_HANDLE};
    VkAllocationCallbacks* allocator{nullptr};
    VkSurfaceKHR surface{VK_NULL_HANDLE};
    
#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debug_messenger;    
      
#endif        
    vulkan_device device;

    vulkan_swapchain swapchain;
    vulkan_renderpass main_renderpass;

    u32 image_index{UINT32_MAX};
    u32 current_frame{UINT32_MAX};
    bool recreating_swapchain{false};
    i32 (*find_memory_index)(u32 type_filter, u32 property_flags);
};