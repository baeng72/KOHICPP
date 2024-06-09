#include "vulkan_backend.hpp"

#include "vulkan_types.inl"
#include "vulkan_utils.hpp"


#include "core/logger.hpp"
#include "core/kmemory.hpp"
#include "core/kstring.hpp"
#include "core/kmemory.hpp"
#include "core/application.hpp"

#include "containers/darray.hpp"

#include "platform/platform.hpp"

static vulkan_context context;//TODO: class member of vulkan_renderer_backend?
static u32 cached_framebuffer_width = 0;
static u32 cached_framebuffer_height = 0;

i32 find_memory_index(u32 type_filter, u32 property_flags);


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

    application::get_framebuffer_size(&cached_framebuffer_width, &cached_framebuffer_height);
    context.framebuffer_width = (cached_framebuffer_width != 0) ? cached_framebuffer_width : 1280;
    context.framebuffer_height = (cached_framebuffer_height != 0) ? cached_framebuffer_height : 720;
    cached_framebuffer_width = cached_framebuffer_height = 0;

    this->platform_state=platform_state;

    context.find_memory_index = find_memory_index;

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
    if(!context.device.create(&context)){
        KERROR("Failed to create device!");
        return false;
    }

    //Swapchain
    context.swapchain.create(&context,context.framebuffer_width,context.framebuffer_height);

    //renderpass
    context.main_renderpass.create(&context,0.f,0.f,(f32)context.framebuffer_width,(f32)context.framebuffer_height,0.f,0.f,0.2f,1.f,1.f,0);

    //swapchain framebuffers
    context.swapchain.framebuffers.reserve(context.swapchain.image_count);
    regenerate_framebuffers(&context.swapchain,&context.main_renderpass);

    //create command buffers
    create_command_buffers();

    //create sync objects.
    context.image_available_semaphores.reserve(context.swapchain.max_frames_in_flight);
    context.queue_complete_semaphores.reserve(context.swapchain.max_frames_in_flight);
    context.in_flight_fences.reserve(context.swapchain.max_frames_in_flight);

    for(u32 i=0; i < context.swapchain.max_frames_in_flight;++i){
        VkSemaphoreCreateInfo semaphore_ci{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(context.device.logical_device, &semaphore_ci, context.allocator, &context.image_available_semaphores[i]);
        vkCreateSemaphore(context.device.logical_device, &semaphore_ci, context.allocator, &context.queue_complete_semaphores[i]);

        context.in_flight_fences[i].create(&context,true);
    }

    //In flight fences should not yet exist at this point, so clear list.
    //These are stored in ponters because the initial state should be 0, and will be 0 when not in use.
    context.images_in_flight.reserve(context.swapchain.image_count);
    for(u32 i=0;i < context.swapchain.image_count; ++i){
        context.images_in_flight[i] = nullptr;
    }


    KINFO("Vulkan renderer initialized successfully.");
    return true;
}

void vulkan_renderer_backend::shutdown(){

    vkDeviceWaitIdle(context.device.logical_device);

    //sync objects
    for(u32 i=0; i < context.swapchain.max_frames_in_flight; ++i){
        if(context.image_available_semaphores[i]){
            vkDestroySemaphore(context.device.logical_device,context.image_available_semaphores[i],context.allocator);
            context.image_available_semaphores[i] = VK_NULL_HANDLE;
            
        }
        if(context.queue_complete_semaphores[i]){
            vkDestroySemaphore(context.device.logical_device,context.queue_complete_semaphores[i],context.allocator);
            context.queue_complete_semaphores[i] = VK_NULL_HANDLE;
        }

        context.in_flight_fences[i].destroy(&context);
    }
    context.image_available_semaphores.clear();
    context.queue_complete_semaphores.clear();
    context.in_flight_fences.clear();
    context.images_in_flight.clear();
    //Command buffers
    for(u32 i=0; i < context.swapchain.image_count; ++i){
        if(context.graphics_command_buffers[i].handle){
            context.graphics_command_buffers[i].free(&context,context.device.graphics_command_pool);
            context.graphics_command_buffers[i].handle = VK_NULL_HANDLE;
        }
    }
    context.graphics_command_buffers.clear();

    //destroy framebuffers
    for(u32 i=0; i < context.swapchain.image_count; ++i){
        context.swapchain.framebuffers[i].destroy(&context);
    }

    //Renderpass
    context.main_renderpass.destroy(&context);

    //Swapchain
    context.swapchain.destroy(&context);

    context.device.destroy(&context);
    #if defined(_DEBUG)
    KDEBUG("Destroying Vulkan debugger...");
    if(context.debug_messenger){
        PFN_vkDestroyDebugUtilsMessengerEXT func = 
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
        func(context.instance,context.debug_messenger,context.allocator);
    }
    #endif
    vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);
    KDEBUG("Destroying Vulkan Instance...");
    vkDestroyInstance(context.instance,context.allocator);
}

void vulkan_renderer_backend::resized(u16 width, u16 height){
    //Update the "framebuffer size generation", a counter which indicates when the framebuffer size has been updated.
    cached_framebuffer_width = width;
    cached_framebuffer_height = height;
    context.framebuffer_size_generation++;

    KINFO("Vulkan renderer resized: w/h/gen: %i/%i/%llu",width, height, context.framebuffer_size_generation);
}

bool vulkan_renderer_backend::begin_frame(f32 delta_time){
    vulkan_device & device = context.device;
    //Check if recreating swap chain and boot out
    if(context.recreating_swapchain){
        VkResult result = device.wait_idle();
        if(!vulkan_result_is_success(result)){
            KERROR("%s vkDeviceWaitIdle (1) failed: '%s'",__FUNCTION__,vulkan_result_string(result,true));
            return false;
        }
        KINFO("Recreating swapchain, booting.");
        return false;
    }

    //Check if the framebuffer has been resized. If so, a new swapchian must be created.
    if(context.framebuffer_size_generation != context.framebuffer_size_last_generation){
        VkResult result = device.wait_idle();
        if(!vulkan_result_is_success(result)){
            KERROR("%s vkDeviceWaitIdle (2) failed: '%s'",vulkan_result_string(result,true));
            return false;
        }

        //if the swapchain recreation failed (because, for example, the window was minimized).
        //boot out before unsetting the flags.
        if(!recreate_swapchain()){
            return false;
        }
        KINFO("Resized, booting");
        return false;
    }

    //Wait for the execution of the current frame to complete. The fence being free willl aloww this one to move  on.
    if(!context.in_flight_fences[context.current_frame].wait(&context,UINT64_MAX)){
        return false;
    }

    //Acquire the next image from the swapchain. Pass along the semaphore that should be signaled when this completes.
    //This same semaphore will later be waited on by the queue submisiosion to ensure this image is available.
    if(!context.swapchain.acquire_next_image_index(&context,UINT64_MAX,
        context.image_available_semaphores[context.current_frame],
        VK_NULL_HANDLE, &context.image_index)){
            return false;
    }

    //Begin recording commands.
    vulkan_command_buffer * command_buffer = &context.graphics_command_buffers[context.image_index];
    command_buffer->reset();
    command_buffer->begin(false,false,false);

    //Dynamic state
    VkViewport viewport;
    viewport.x = 0.f;
    viewport.y = (f32)context.framebuffer_height;
    viewport.width = (f32)context.framebuffer_width;
    viewport.height = -(f32)context.framebuffer_height;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    //Scissor
    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = context.framebuffer_width;
    scissor.extent.height = context.framebuffer_height;

    vkCmdSetViewport(command_buffer->handle,0,1,&viewport);
    vkCmdSetScissor(command_buffer->handle,0,1,&scissor);

    context.main_renderpass.w = (f32)context.framebuffer_width;
    context.main_renderpass.h = (f32)context.framebuffer_height;

    //Begin the renderpass
    context.main_renderpass.begin(command_buffer,context.swapchain.framebuffers[context.image_index]);


    return true;
}

bool vulkan_renderer_backend::end_frame(f32 delta_time){
    vulkan_command_buffer * command_buffer = &context.graphics_command_buffers[context.image_index];
    context.main_renderpass.end(command_buffer);
    command_buffer->end();

    //Make sure the previous frame is not using this image (i.e. it's fence is being waited on)
    if(context.images_in_flight[context.image_index] != VK_NULL_HANDLE){
        context.images_in_flight[context.image_index]->wait(&context,UINT64_MAX);
    }

    //Mark the image fence as in-use by this frame
    context.images_in_flight[context.image_index] = &context.in_flight_fences[context.current_frame];

    //REset the fence for use on the next frame
    context.in_flight_fences[context.current_frame].reset(&context);

    //Submit the queue and wait for the operation to complete.
    //Begin queue submission
    VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};

    //Command buffer(s) to be executed.
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer->handle;

    //The semaphore(s) to be signaled when the queue is complete
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &context.queue_complete_semaphores[context.current_frame];

    //Wait semaphore ensures that the operation cannot begin until the image is available.
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &context.image_available_semaphores[context.current_frame];

    //Each semaphore waits on the corresponding pipeline stage to complete. 1:1 ration.
    //VK_PIPELNE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent color attachment
    //writes from executing until the semaphore signals (i.e. one from is presented at a time)
    VkPipelineStageFlags flags[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.pWaitDstStageMask = flags;

    VkResult result = vkQueueSubmit(
        context.device.graphics_queue, 1, &submit_info, context.in_flight_fences[context.current_frame].handle);
    if(result != VK_SUCCESS){
        KERROR("vkQueueSubmit failed with result: %s", vulkan_result_string(result, true));
        return false;
    }
    
    command_buffer->update_submitted();

    //Give the image back to swpachian
    context.swapchain.present(&context, context.device.graphics_queue, context.device.present_queue,
        context.queue_complete_semaphores[context.current_frame],context.image_index);

    return true;
}

void* vulkan_renderer_backend::operator new(size_t size){
    void* ptr = kallocate(sizeof(vulkan_renderer_backend),MEMORY_TAG_RENDERER);
    return ptr;
}

void vulkan_renderer_backend::operator delete(void * ptr){
    kfree(ptr, sizeof(vulkan_renderer_backend),MEMORY_TAG_RENDERER);
}

void vulkan_renderer_backend::create_command_buffers(){
    if(!context.graphics_command_buffers.length()){
        context.graphics_command_buffers.reserve(context.swapchain.image_count);
        for(u32 i=0; i < context.swapchain.image_count; ++i){
            kzero_memory(&context.graphics_command_buffers[i],sizeof(vulkan_command_buffer));
        }
    }

    for(u32 i=0; i < context.swapchain.image_count; ++i){
        if(context.graphics_command_buffers[i].handle){
            context.graphics_command_buffers[i].free(&context,context.device.graphics_command_pool);
        }
        kzero_memory(&context.graphics_command_buffers[i],sizeof(vulkan_command_buffer));
        context.graphics_command_buffers[i].allocate(&context,context.device.graphics_command_pool,true);
    }
    KDEBUG("Vulkan command buffers created.");
}

void vulkan_renderer_backend::regenerate_framebuffers(vulkan_swapchain*swapchain, vulkan_renderpass*renderpass){
    for(u32 i=0; i < swapchain->image_count; ++i){
        u32 attachment_count=2;
        VkImageView attachments[] =  {
            swapchain->views[i],
            swapchain->depth_attachment.view
        };

        context.swapchain.framebuffers[i].create(&context,renderpass,
            context.framebuffer_width,context.framebuffer_height,
            attachment_count, attachments);
    }
}

bool vulkan_renderer_backend::recreate_swapchain(){
    //If already being created, don't try again.
    if(context.recreating_swapchain){
        KDEBUG("%s called when already recreating. Booting.",__FUNCTION__);
        return false;
    }

    //Detect if the window is too small to be drawn to
    if(context.framebuffer_width==0 || context.framebuffer_height == 0){
        KDEBUG("%s called when window is < 1 in a dimension. Booting.",__FUNCTION__);
        return false;
    }

    //Mark as recreating if the dimensions are valid.
    context.recreating_swapchain = true;

    //Wait for any opertions to complete
    context.device.wait_idle();

    //Clear these out just in case
    for(u32 i=0; i < context.swapchain.image_count; ++i){
        context.images_in_flight[i] = nullptr;
    }

    //Requery support
    context.device.query_swapchain_support(context.device,context.surface,&context.device.swapchain_support);
    context.device.detect_depth_format();

    context.swapchain.recreate(&context,cached_framebuffer_width, cached_framebuffer_height);

    //Sync the framebuffer sie with the cached sizes
    context.framebuffer_width = cached_framebuffer_width;
    context.framebuffer_height = cached_framebuffer_height;
    context.main_renderpass.w = (f32)context.framebuffer_width;
    context.main_renderpass.h = (f32)context.framebuffer_height;
    cached_framebuffer_width = 0;
    cached_framebuffer_height = 0;

    //Update framebuffer size generation.
    context.framebuffer_size_last_generation = context.framebuffer_size_generation;

    //cleanup swapchian
    for(u32 i=0; i < context.swapchain.image_count; ++i){
        context.graphics_command_buffers[i].free(&context,context.device.graphics_command_pool);
    }

    //framebuffers
    for(u32 i=0; i < context.swapchain.image_count; ++i){
        context.swapchain.framebuffers[i].destroy(&context);
    }

    context.main_renderpass.x = 0;
    context.main_renderpass.y = 0;
    context.main_renderpass.w = (f32)context.framebuffer_width;
    context.main_renderpass.h = (f32)context.framebuffer_height;

    regenerate_framebuffers(&context.swapchain,&context.main_renderpass);

    create_command_buffers();

    //Clear the regenerating flag
    context.recreating_swapchain = false;

    return true;
}

#if defined(_DEBUG)


void SetResourceName(vulkan_context*context,VkObjectType type, u64 handle, ccharp name){
    context->device.SetResourceName(type,handle,name);
    
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

i32 find_memory_index(u32 type_filter, u32 property_flags){
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(context.device.physical_device, &memory_properties);

    for(u32 i=0; i < memory_properties.memoryTypeCount; ++ i){
        if(type_filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags){
            return i;
        }
    }

    KWARN("Unable to find suitable memory type!");
    return -1;
}



#endif