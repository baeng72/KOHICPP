#include "vulkan_framebuffer.hpp"

#include "core/kmemory.hpp"

void vulkan_framebuffer::create(vulkan_context*context, vulkan_renderpass*renderpass_, u32 width, u32 height,
    u32 attachment_count_, VkImageView* attachments_){
    

    //Take a copy of the attachments, renderpass and attachment count
    attachment_count = attachment_count_;
    attachments = (VkImageView*)kallocate(sizeof(VkImageView)*attachment_count_,MEMORY_TAG_RENDERER);
    for(u32 i=0; i < attachment_count; ++i){
        attachments[i] = attachments_[i];
    }

    renderpass = renderpass_;

    //Creation info
    VkFramebufferCreateInfo framebuffer_create_info{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    framebuffer_create_info.renderPass = renderpass->handle;
    framebuffer_create_info.attachmentCount = attachment_count;
    framebuffer_create_info.pAttachments = attachments;
    framebuffer_create_info.width = width;
    framebuffer_create_info.height =  height;
    framebuffer_create_info.layers = 1;

    VK_CHECK(vkCreateFramebuffer(context->device.logical_device, &framebuffer_create_info, context->allocator,&handle));

}

void vulkan_framebuffer::destroy(vulkan_context* context){
    vkDestroyFramebuffer(context->device.logical_device, handle, context->allocator);
    if(attachments){
        kfree(attachments, sizeof(VkImageView)*attachment_count, MEMORY_TAG_RENDERER);
        attachments=nullptr;
    }

    handle = VK_NULL_HANDLE;
    attachment_count = 0;
    renderpass = nullptr;
}