#pragma once

#include "core/application.hpp"

struct application_state;
struct game{
    application_config app_config;

    application_state* application_state{nullptr};

    virtual bool initialize()=0;

    virtual bool update(f32 delta_time)=0;
    
    virtual bool render(f32 delta_time)=0;
    
    virtual void on_resize(u32 width, u32 height)=0;  
};