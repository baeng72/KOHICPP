#include "vulkan_backend.hpp"

#include "vulkan_types.inl"

#include "core/logger.hpp"
#include "core/kmemory.hpp"

static vulkan_context context;//TODO: class member of vulkan_renderer_backend?

vulkan_renderer_backend::vulkan_renderer_backend(){

}

vulkan_renderer_backend::~vulkan_renderer_backend(){

}

bool vulkan_renderer_backend::initialize(ccharp application_name,platform*platform_state){
    context.allocator = nullptr;

    this->platform_state=platform_state;

    //Setup vulkan instance
    VkApplicationInfo app_info{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.apiVersion = VK_API_VERSION_1_2;
    app_info.pApplicationName = application_name;
    app_info.applicationVersion = VK_MAKE_VERSION(1,0,0);
    app_info.pEngineName = "Kohi CPP Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo create_info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = 0;
    create_info.ppEnabledExtensionNames = nullptr;
    create_info.enabledLayerCount = 0;
    create_info.ppEnabledLayerNames = nullptr;

    VkResult result = vkCreateInstance(&create_info, context.allocator, &context.instance);
    if(result != VK_SUCCESS){
        KERROR("vkCreateInstance failed with result: %u\n",result);
        return false;
    }
    KINFO("Vulkan renderer initialized successfully.\n");
    return true;
}

void vulkan_renderer_backend::shutdown(){
    vkDestroyInstance(context.instance,context.allocator);
}

void vulkan_renderer_backend::resized(u16 width, u16 height){

}

bool vulkan_renderer_backend::begin_frame(f32 delta_time){
    return true;
}

bool vulkan_renderer_backend::end_frame(f32 delta_time){
    return true;
}

void* vulkan_renderer_backend::operator new(size_t size){
    void* ptr = kallocate(sizeof(vulkan_renderer_backend),MEMORY_TAG_RENDERER);
    return ptr;
}

void vulkan_renderer_backend::operator delete(void * ptr){
    kfree(ptr, sizeof(vulkan_renderer_backend),MEMORY_TAG_RENDERER);
}