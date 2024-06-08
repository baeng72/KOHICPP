#include "vulkan_swapchain.hpp"

#include "core/logger.hpp"
#include "core/kstring.hpp"
#include "vulkan_device.hpp"
#include "vulkan_image.hpp"

void create_swapchain(vulkan_context* context, u32 width, u32 height, vulkan_swapchain*swapchain);
void destroy_swapchain(vulkan_context* context, vulkan_swapchain * swapchain);

void vulkan_swapchain::create(vulkan_context* context, u32 width, u32 height){
    create_swapchain(context,width, height, this);
}

void vulkan_swapchain::recreate(vulkan_context* context, u32 width, u32 height){
    //destroy the old and create a new one
    destroy_swapchain(context, this);
    create_swapchain(context, width, height, this);
}

void vulkan_swapchain::destroy(vulkan_context*context){
    destroy_swapchain(context,this);
}

bool vulkan_swapchain::acquire_next_image_index(vulkan_context* context, u64 timeout_ns, VkSemaphore image_available_semaphore, VkFence fence,u32*out_image_index){
    VkResult result = vkAcquireNextImageKHR(context->device.logical_device,
        handle, timeout_ns, image_available_semaphore, fence, out_image_index);
    
    if(result == VK_ERROR_OUT_OF_DATE_KHR){
        //Trigger swapchain recreation, the boot out of the render loop.
        recreate(context,context->framebuffer_width,context->framebuffer_height);
        return false;
    }else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
        KFATAL("Failed to acquire swapchain image!");
        return false;
    }
    return true;
}

void vulkan_swapchain::present(vulkan_context* context, VkQueue graphics_queue, VkQueue present_queue,
    VkSemaphore render_complete_semaphore, u32 present_image_index){

    //Return the image to the swapchain for presentation.
    VkPresentInfoKHR present_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &render_complete_semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &handle;
    present_info.pImageIndices = &present_image_index;
    present_info.pResults = 0;
}

void create_swapchain(vulkan_context* context, u32 width, u32 height, vulkan_swapchain * swapchain){
    VkExtent2D swapchain_extent = {width, height};
    swapchain->max_frames_in_flight = 2;

    //Choose a swap surface format.
    bool found = false;
    for(u32 i = 0; i< context->device.swapchain_support.format_count; ++ i){
        VkSurfaceFormatKHR format = context->device.swapchain_support.formats[i];
        //Preferred formats
        if(format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
            swapchain->image_format = format;
            found = true;
        }
    }

    if(!found){
        swapchain->image_format = context->device.swapchain_support.formats[0];
    }


    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for(u32 i=0; i < context->device.swapchain_support.present_mode_count; ++i){
        VkPresentModeKHR mode = context->device.swapchain_support.present_modes[i];
        if(mode == VK_PRESENT_MODE_MAILBOX_KHR){
            present_mode = mode;
            break;
        }
    }

    //Requerty swapchain support
    context->device.query_swapchain_support(context->device.physical_device,
        context->surface,
        &context->device.swapchain_support);

    //swapchain extent
    if(context->device.swapchain_support.capabilities.currentExtent.width != UINT32_MAX){
        swapchain_extent = context->device.swapchain_support.capabilities.currentExtent;
    }

    //Clamp to the value allowed by the GPU.
    VkExtent2D min = context->device.swapchain_support.capabilities.minImageExtent;
    VkExtent2D max = context->device.swapchain_support.capabilities.maxImageExtent;
    swapchain_extent.width = KCLAMP(swapchain_extent.width, min.width, max.width);
    swapchain_extent.height = KCLAMP(swapchain_extent.height, min.height, max.height);

    u32 image_count = context->device.swapchain_support.capabilities.minImageCount + 1;
    if(context->device.swapchain_support.capabilities.maxImageCount > 0 && image_count > context->device.swapchain_support.capabilities.maxImageCount){
        image_count = context->device.swapchain_support.capabilities.maxImageCount;
    }

    //Swapchain create info
    VkSwapchainCreateInfoKHR swapchain_create_info{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    swapchain_create_info.surface = context->surface;
    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.imageFormat = swapchain->image_format.format;
    swapchain_create_info.imageColorSpace = swapchain->image_format.colorSpace;
    swapchain_create_info.imageExtent = swapchain_extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    //Setup the queue family indices
    if(context->device.graphics_queue_index != context->device.present_queue_index){
        u32 queueFamilyIndices[] = {
            (u32)context->device.graphics_queue_index,
            (u32)context->device.present_queue_index
        };

        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = queueFamilyIndices;
    }

    swapchain_create_info.preTransform = context->device.swapchain_support.capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = nullptr;

    VK_CHECK(vkCreateSwapchainKHR(context->device.logical_device, &swapchain_create_info, context->allocator, &swapchain->handle));

    //Views
    for(u32 i = 0; i < swapchain->image_count; ++i){
        VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        view_info.image = swapchain->images[i];
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = swapchain->image_format.format;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(context->device.logical_device, &view_info, context->allocator, &swapchain->views[i]));
    }

    //Depth resources
    if(!context->device.detect_depth_format()){
        context->device.depth_format = VK_FORMAT_UNDEFINED;
        KFATAL("Failed to find a supported format!");
    }

    //Create depth image an its view.
    swapchain->depth_attachment.create(
        context,
        VK_IMAGE_TYPE_2D,
        swapchain_extent.width,
        swapchain_extent.height,
        context->device.depth_format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        true,
        VK_IMAGE_ASPECT_DEPTH_BIT);
    

    KINFO("Swapchain created successfully.");


}

void destroy_swapchain(vulkan_context * context, vulkan_swapchain * swapchain){
    swapchain->depth_attachment.destroy(context);

    //Only destroy the views, not the images, since those are owned by the swapchain and are thus
    //destroy when it is.
    for(u32 i=0; i < swapchain->image_count; ++i){
        vkDestroyImageView(context->device.logical_device, swapchain->views[i], context->allocator);
    }

    vkDestroySwapchainKHR(context->device.logical_device, swapchain->handle, context->allocator);
}