#pragma once

#include "defines.hpp"

enum renderer_backend_type{
    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_OPENGL,
    RENDERER_BACKEND_TYPE_DIRECTX
};
class KAPI platform;
class renderer_backend{
protected:
    u64 frame_number;
    platform*platform_state;
public:
    renderer_backend(){}
    virtual ~renderer_backend()=default;
    virtual bool initialize(ccharp application_name,platform*platform_state)=0;
    virtual void shutdown()=0;
    virtual bool begin_frame(f32 delta_time)=0;
    virtual bool end_frame(f32 delta_time)=0;
    virtual void resized(u16 width, u16 height)=0;
    
};

struct renderer_packet{
    f32 delta_time;
};