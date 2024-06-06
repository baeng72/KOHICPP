#include "vulkan_backend.hpp"

#include "vulkan_types.inl"


#include "core/logger.hpp"
#include "core/kmemory.hpp"
#include "core/kstring.hpp"

#include "containers/darray.hpp"

#include "platform/platform.hpp"

static vulkan_context context;//TODO: class member of vulkan_renderer_backend?

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void * user_data);

void SetResourceName(VkDevice device, VkObjectType type, u64 handle, ccharp name);
void BeginLabel(VkCommandBuffer cmd);
void EndLabel(VkCommandBuffer cmd);    

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

    //obtain list of required extensions
    darray<ccharp> extensions;
    platform_state->get_required_extensions_names(extensions);
    #if defined(_DEBUG)
    extensions.push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    extensions.push(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    KDEBUG("Required extensions:");
    u32 length = (u32)extensions.length();
    for(u32 i=0; i < length; i++){
        KDEBUG(extensions[i]);
    }
    #endif
    darray<ccharp> layers;
    #if defined(_DEBUG)
    KINFO("Validation layers enabled. Enumerating...");

    //The list of validation layers required
    layers.push("VK_LAYER_KHRONOS_validation");
    u32 layer_count = (u32)layers.length();

    //obtain a list of available validation layers
    u32 available_layer_count=0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count,nullptr));
    darray<VkLayerProperties> available_layers(available_layer_count);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count,available_layers));

    //verify all required layers are available
    for(u32 i=0; i < layer_count; ++i){
        KINFO("Searching for layer: %s...",layers[i]);
        bool found = false;
        for(u32 j=0; j < available_layer_count; ++j){
            if(strings_equal(layers[i],available_layers[j].layerName)){
                found = true;
                KINFO("Found.");
                break;
            }
        }

        if(!found){
            KFATAL("Required validation layer is missing: %s", layers[i]);
            return false;
        }
    }
    KINFO("ALL required validation layers are present.");
    #endif

    VkInstanceCreateInfo create_info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = (u32)extensions.length();
    create_info.ppEnabledExtensionNames = extensions;
    create_info.enabledLayerCount = (u32)layers.length();
    create_info.ppEnabledLayerNames = layers;

    VK_CHECK(vkCreateInstance(&create_info, context.allocator, &context.instance));
    KINFO("Vulkan instance created.");
    
    //debugger
    #if defined(_DEBUG)
    KDEBUG("Creating Vulkan debugger...");
    u32 log_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debug_create_info.messageSeverity = log_severity;
    debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;                 
    debug_create_info.pfnUserCallback = vk_debug_callback;

    PFN_vkCreateDebugUtilsMessengerEXT func = 
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkCreateDebugUtilsMessengerEXT");
    KASSERT_MSG(func, "Failed to create debug messenger!");
    VK_CHECK(func(context.instance, &debug_create_info, context.allocator, &context.debug_messenger));
    KDEBUG("Vulkan debugger created.");
    #endif

    //Surface
    KDEBUG("Creating Vulkan surface...");
    if(!platform_state->create_vulkan_surface(&context)){
        KERROR("Failed to create platform surface!");
        return false;
    }
    KDEBUG("Vulkan surface created.");

    //Device creation
    if(!context.device.create(context.instance,context.surface)){
        KERROR("Failed to create device!");
        return false;
    }
    KINFO("Vulkan renderer initialized successfully.");
    return true;
}

void vulkan_renderer_backend::shutdown(){
    context.device.destroy(context.instance);
    #if defined(_DEBUG)
    KDEBUG("Destroying Vulkan debugger...");
    if(context.debug_messenger){
        PFN_vkDestroyDebugUtilsMessengerEXT func = 
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
        func(context.instance,context.debug_messenger,context.allocator);
    }
    #endif
    KDEBUG("Destroying Vulkan Instance...");
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
#if defined(_DEBUG)


void SetResourceName(VkDevice device, VkObjectType type, u64 handle, ccharp name){
    VkDebugUtilsObjectNameInfoEXT ni{ VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
    ni.objectType = type;
    ni.objectHandle = handle;
    ni.pObjectName = name;
    context.pfnSetObjectName(device, &ni);
}

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data) {
    switch (message_severity) {
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            KERROR(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            KWARN(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            KINFO(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            KTRACE(callback_data->pMessage);
            break;
    }
    return VK_FALSE;
}

#endif