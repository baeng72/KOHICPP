#include "vulkan_image.hpp"

#include "vulkan_device.hpp"

#include "core/kmemory.hpp"
#include "core/logger.hpp"

void vulkan_image::create(vulkan_context*context, VkImageType image_type, u32 w, u32 h,
    VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_flags,
    bool create_view, VkImageAspectFlags view_aspect_flags){

    //copy params
    width = w;
    height = h;

    //Creation info
    VkImageCreateInfo image_create_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = w;
    image_create_info.extent.height = h;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 4;
    image_create_info.arrayLayers = 1;
    image_create_info.format = format;
    image_create_info.tiling = tiling;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = usage;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateImage(context->device.logical_device, &image_create_info, context->allocator,&handle));

    //Query memory requirements
    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(context->device.logical_device, handle, &memory_requirements);

    i32 memory_type= context->find_memory_index(memory_requirements.memoryTypeBits, memory_flags);
    if(memory_type==-1){
        KERROR("Required memory type not found. Image not valid.");
    }

    //Allocate memory
    VkMemoryAllocateInfo memory_allocate_info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    memory_allocate_info.allocationSize = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = memory_type;
    VK_CHECK(vkAllocateMemory(context->device.logical_device,&memory_allocate_info, context->allocator, &memory));

    //Bind the memory
    VK_CHECK(vkBindImageMemory(context->device.logical_device, handle, memory, 0));

    //create view
    if(create_view){
        view = nullptr;
        create_image_view(context, format,view_aspect_flags);
    }
}

void vulkan_image::create_image_view(vulkan_context*context, VkFormat format, VkImageAspectFlags aspect_flags){

    VkImageViewCreateInfo view_create_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_create_info.image = handle;
    view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_create_info.format = format;
    view_create_info.subresourceRange.aspectMask = aspect_flags;
    view_create_info.subresourceRange.baseMipLevel = 0;
    view_create_info.subresourceRange.levelCount = 1;
    view_create_info.subresourceRange.baseArrayLayer = 0;
    view_create_info.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(context->device.logical_device, &view_create_info, context->allocator, &view));
}

void vulkan_image::destroy(vulkan_context*context){
    if(view){
        vkDestroyImageView(context->device.logical_device,view,context->allocator);
        view=VK_NULL_HANDLE;
    }
    if(memory){
        vkFreeMemory(context->device.logical_device, memory, context->allocator);
        memory = VK_NULL_HANDLE;
    }

    if(handle){
        vkDestroyImage(context->device.logical_device, handle, context->allocator);
        handle = VK_NULL_HANDLE;
    }
}
