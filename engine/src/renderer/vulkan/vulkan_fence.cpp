#include "vulkan_fence.hpp"

#include "core/logger.hpp"

void vulkan_fence::create(vulkan_context* context, bool create_signaled){
    //Make sure to signal the fence if required.
    is_signaled = create_signaled;
    VkFenceCreateInfo fence_create_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fence_create_info.flags = create_signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
    VK_CHECK(vkCreateFence(context->device.logical_device,&fence_create_info,context->allocator,&handle));
}

void vulkan_fence::destroy(vulkan_context*context){
    if(handle){
        vkDestroyFence(context->device.logical_device,handle,context->allocator);
        handle=VK_NULL_HANDLE;
    }
    is_signaled = false;
}

bool vulkan_fence::wait(vulkan_context* context, u64 timeout_ms){
    if(!is_signaled){
        VkResult result = vkWaitForFences(context->device.logical_device,1,&handle,VK_TRUE,timeout_ms);
        switch(result){
            case VK_SUCCESS:
                is_signaled=true;
                break;
                case VK_TIMEOUT:
                KWARN("%s - timed out",__FUNCTION__);
                break;
                case VK_ERROR_DEVICE_LOST:
                KERROR("%s - VK_ERROR_DEVICE_LOST.",__FUNCTION__);
                break;
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                KERROR("%s - VK_ERROR_OUT_OF_HOST_MEMORY.",__FUNCTION__);
                break;
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                KERROR("%s - VK_ERROR_OUT_OF_DEVICE_MEMORY.",__FUNCTION__);
                break;
                default:
                KERROR("%s - An unknown error has occurred.",__FUNCTION__);
                break;
        }
    }else{
        //if already signaled, don't wait
        return true;
    }
    return false;
}

void vulkan_fence::reset(vulkan_context*context){
    if(is_signaled){
        VK_CHECK(vkResetFences(context->device.logical_device,1,&handle));
        is_signaled=false;
    }
}