#include "vulkan_command_buffer.hpp"
#include "core/kmemory.hpp"

void vulkan_command_buffer::allocate(vulkan_context*context, VkCommandPool pool, bool is_primary){
    handle = VK_NULL_HANDLE;
    state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
    VkCommandBufferAllocateInfo allocate_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocate_info.commandPool = pool;
    allocate_info.level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocate_info.commandBufferCount = 1;
    allocate_info.pNext = nullptr;

    VK_CHECK(vkAllocateCommandBuffers(context->device.logical_device,
        &allocate_info,&handle));

    state = COMMAND_BUFFER_STATE_READY;

}

void vulkan_command_buffer::free(
    vulkan_context* context,
    VkCommandPool pool){
    
    vkFreeCommandBuffers(context->device.logical_device, pool, 1, &handle);

    handle = VK_NULL_HANDLE;
    state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
}

void vulkan_command_buffer::begin(bool is_single_use, bool is_renderpass_continue, bool is_simultaneous_use){
    VkCommandBufferBeginInfo begin_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin_info.flags = 0;
    if(is_single_use)
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if(is_renderpass_continue)
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

    if(is_simultaneous_use)
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    VK_CHECK(vkBeginCommandBuffer(handle, &begin_info));

    state = COMMAND_BUFFER_STATE_RECORDING;
}

void vulkan_command_buffer::end(){
    VK_CHECK(vkEndCommandBuffer(handle));
    state = COMMAND_BUFFER_STATE_RECORDING_ENDED;
}

void vulkan_command_buffer::update_submitted(){
    state = COMMAND_BUFFER_STATE_SUBMITTED;
}

void vulkan_command_buffer::reset(){
    state = COMMAND_BUFFER_STATE_READY;
}

void vulkan_command_buffer::allocate_and_begin_single_use(vulkan_context*context, VkCommandPool pool){
    allocate(context,pool,true);
    begin(true,false,false);
}

void vulkan_command_buffer::end_single_use(vulkan_context*context, VkCommandPool pool, VkQueue queue){
    //End the command bufffer
    end();

    //Fence
    VkFenceCreateInfo fence_ci{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    
    VkFence fence{VK_NULL_HANDLE};
    VK_CHECK(vkCreateFence(context->device.logical_device,&fence_ci,context->allocator,&fence));

    //Submit the queue
    VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.commandBufferCount=1;
    submit_info.pCommandBuffers = &handle;
    
    VK_CHECK(vkQueueSubmit(queue,1,&submit_info,fence));

    vkWaitForFences(context->device.logical_device,1,&fence,VK_TRUE,UINT64_MAX);
    vkDestroyFence(context->device.logical_device,fence,context->allocator);

    //Free the command buffer
    free(context,pool);
}
