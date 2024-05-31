#include "renderer_frontend.hpp"
#include "renderer_backend.hpp"
#include "core/logger.hpp"

bool renderer::initialize(ccharp application_name,platform*platform_state){
    backend = renderer_backend_create(RENDERER_BACKEND_TYPE_VULKAN);
    if(!backend || !backend->initialize(application_name,platform_state)){
        KFATAL("Renderer backend failure to initialize. Shutting down.");
        return false;
    }
    return true;
}

void renderer::shutdown(){
    if(backend){
        backend->shutdown();
        delete backend;
        backend=nullptr;
    }
}

bool renderer::begin_frame(f32 delta_time){
    return backend->begin_frame(delta_time);
}

bool renderer::end_frame(f32 delta_time){
    return backend->end_frame(delta_time);
}

bool renderer::draw_frame(renderer_packet*packet){
    if(begin_frame(packet->delta_time)){
        bool result = end_frame(packet->delta_time);
        if(!result){
            KERROR("renderer_end_frame failed. Application shutting down...");
            return false;
        }
    }
    return true;
}