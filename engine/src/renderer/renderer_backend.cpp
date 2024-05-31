#include "renderer_backend.hpp"
#include "vulkan/vulkan_backend.hpp"

renderer_backend* renderer_backend_create(renderer_backend_type type){
    switch(type){
        case RENDERER_BACKEND_TYPE_VULKAN:
        return new vulkan_renderer_backend();
        case RENDERER_BACKEND_TYPE_OPENGL:
        case RENDERER_BACKEND_TYPE_DIRECTX:
        break;
    }
    return nullptr;
}