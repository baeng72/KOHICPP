#pragma once

#include "renderer/renderer_backend.hpp"

struct vulkan_swapchain;
struct vulkan_renderpass;

class vulkan_renderer_backend : public renderer_backend{    
    
    public:
    vulkan_renderer_backend();
    virtual ~vulkan_renderer_backend();
    virtual bool initialize(ccharp application_name,platform*platform)override;
    virtual void shutdown()override;
    virtual bool begin_frame(f32 delta_time)override;
    virtual bool end_frame(f32 delta_time)override;
    virtual void resized(u16 width, u16 height)override;
    void create_command_buffers();
    void regenerate_framebuffers(vulkan_swapchain*swapchain, vulkan_renderpass * renderpass);
    bool recreate_swapchain();
    void * operator new(size_t);
    void operator delete(void*);
};