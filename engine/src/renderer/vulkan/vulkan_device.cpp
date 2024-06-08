#include "vulkan_device.hpp"
#include "core/logger.hpp"
#include "core/kstring.hpp"
#include "core/kmemory.hpp"
#include "containers/darray.hpp"

struct vulkan_physical_device_requirements{
    bool graphics{false};
    bool present{false};
    bool compute{false};
    bool transfer{false};
    //darray
    darray<ccharp> device_extensions_names;
    bool sampler_anisotropy{false};
    bool discrete_gpu{false};
};

struct vulkan_physical_device_queue_family_info{
    u32 graphics_family_index{UINT32_MAX};
    u32 present_family_index{UINT32_MAX};
    u32 compute_family_index{UINT32_MAX};
    u32 transfer_family_index{UINT32_MAX};
};


bool vulkan_device::create(vulkan_context*context){
    if(!select_physical_device(context->instance,context->surface))
        return false;

    KINFO("Creating logical device...");
    //NOTE: don't create additional queues for shared indices
    bool present_shares_graphics_queue =  context->device.graphics_queue_index == context->device.present_queue_index;
    bool transfer_shares_graphics_queue = context->device.transfer_queue_index == context->device.graphics_queue_index;

    u32 index_count=1;
    if(!present_shares_graphics_queue){
        index_count++;
    }
    if(!transfer_shares_graphics_queue){
        index_count++;
    }

    darray<u32> indices(index_count);
    u32 index=0;
    indices[index++] = context->device.graphics_queue_index;
    if(!present_shares_graphics_queue){
        indices[index++] = context->device.present_queue_index;
    }
    if(!transfer_shares_graphics_queue){
        indices[index++] = context->device.transfer_queue_index;
    }

    darray<VkDeviceQueueCreateInfo> queue_create_infos(index_count);
    f32 queue_priorities[2]={1.f,1.f};
    for(u32 i = 0; i < index_count; ++i){
        queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[i].queueFamilyIndex = indices[i];
        queue_create_infos[i].queueCount = 1;
        if(indices[i] == context->device.graphics_queue_index){
            queue_create_infos[i].queueCount = 2;
        }
        queue_create_infos[i].flags = 0;
        queue_create_infos[i].pNext = 0;
        queue_create_infos[i].pQueuePriorities = queue_priorities;
    }

    VkPhysicalDeviceFeatures device_features{};
    device_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo device_create_info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    device_create_info.queueCreateInfoCount = index_count;
    device_create_info.pQueueCreateInfos = queue_create_infos;
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = 1;
    ccharp extension_names = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    device_create_info.ppEnabledExtensionNames = &extension_names;
    device_create_info.enabledLayerCount=0;
    device_create_info.ppEnabledLayerNames = nullptr;

    VK_CHECK(vkCreateDevice(
        physical_device,
        &device_create_info,
        context->allocator,
        &logical_device));

    KINFO("Logical device created.");

    //Get queues
    vkGetDeviceQueue(logical_device, context->device.graphics_queue_index,0,&context->device.graphics_queue);
    vkGetDeviceQueue(logical_device, context->device.present_queue_index, 0 ,&context->device.present_queue);
    vkGetDeviceQueue(logical_device, context->device.transfer_queue_index, 0, &context->device.transfer_queue);

    pfnBeginLabel = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetDeviceProcAddr(logical_device,"vkCmdBeginDebugUtilsLabelEXT");
    pfnEndLabel = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetDeviceProcAddr(logical_device,"vkCmdEndDebugUtilsLabelEXT");
    pfnSetObjectName = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(logical_device,"vkSetDebugUtilsObjectNameEXT");

    KINFO("Queues obtained.");

    //Create command pool for graphics queue.
    VkCommandPoolCreateInfo pool_create_info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK(vkCreateCommandPool(
        logical_device, &pool_create_info, context->allocator,&graphics_command_pool));
    KINFO("Graphics command pool created.");

    return true;
}

void vulkan_device::destroy(vulkan_context*context){
    //unset queues
    context->device.graphics_queue = VK_NULL_HANDLE;
    context->device.present_queue = VK_NULL_HANDLE;
    context->device.graphics_queue = VK_NULL_HANDLE;

    KINFO("Destroy command pools...");
    vkDestroyCommandPool(
        logical_device,
        graphics_command_pool,
        context->allocator);
    

    //Destroy logical device
    KINFO("Destroying logical device...");
    if(logical_device){
        vkDestroyDevice(logical_device, context->allocator);
        logical_device = VK_NULL_HANDLE;
    }

    //Physical devices are not destroyed
    KINFO("Releasing physical device resources...");
    physical_device = VK_NULL_HANDLE;

    if(context->device.swapchain_support.formats){
        kfree(context->device.swapchain_support.formats,
            sizeof(VkSurfaceFormatKHR)*context->device.swapchain_support.format_count,
            MEMORY_TAG_RENDERER);
        context->device.swapchain_support.formats = nullptr;
        context->device.swapchain_support.format_count = 0;
    }

    if(context->device.swapchain_support.present_modes){
        kfree(context->device.swapchain_support.present_modes,
            sizeof(VkPresentModeKHR)*context->device.swapchain_support.present_mode_count,
            MEMORY_TAG_RENDERER);
        context->device.swapchain_support.present_modes = nullptr;
        context->device.swapchain_support.present_mode_count = 0;
    }

    kzero_memory(&context->device.swapchain_support.capabilities,sizeof(VkSurfaceCapabilitiesKHR));
    context->device.graphics_queue_index=-1;
    context->device.present_queue_index=-1;
    context->device.transfer_queue_index=-1;
}

void vulkan_device::query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface , vulkan_swapchain_support_info*out_support_info){

    //surface capabilities
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &out_support_info->capabilities));

    //Surface formats
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface,&out_support_info->format_count,nullptr));

    if(out_support_info->format_count != 0){
        if(!out_support_info->formats)
            out_support_info->formats = (VkSurfaceFormatKHR*)kallocate(sizeof(VkSurfaceFormatKHR) * out_support_info->format_count, MEMORY_TAG_RENDERER);
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &out_support_info->format_count, out_support_info->formats));
    }

    //Present modes
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
        &out_support_info->present_mode_count,nullptr));

    if(out_support_info->present_mode_count != 0){
        if(!out_support_info->present_modes){
            out_support_info->present_modes = (VkPresentModeKHR*)kallocate(sizeof(VkPresentModeKHR)* out_support_info->present_mode_count,MEMORY_TAG_RENDERER);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &out_support_info->present_mode_count,
            out_support_info->present_modes));
    }        
}

bool vulkan_device::select_physical_device(VkInstance instance, VkSurfaceKHR surface){
    u32 physical_device_count = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(instance,&physical_device_count,nullptr));
    if(physical_device_count == 0){
        KFATAL("No devices which support Vulkan were found.");
        return false;
    }

    darray<VkPhysicalDevice> physical_devices(physical_device_count);
    VK_CHECK(vkEnumeratePhysicalDevices(instance,&physical_device_count,physical_devices));

    for(u32 i=0; i < physical_device_count; ++i){
        VkPhysicalDeviceProperties curr_props;
        vkGetPhysicalDeviceProperties(physical_devices[i],&curr_props);

        VkPhysicalDeviceFeatures curr_features;
        vkGetPhysicalDeviceFeatures(physical_devices[i],&curr_features);

        VkPhysicalDeviceMemoryProperties curr_memory;
        vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &curr_memory);

        //TODO: These requirements should probably be driven by engine
        //configuration.
        vulkan_physical_device_requirements requirements{};
        requirements.graphics=true;
        requirements.present=true;
        requirements.transfer = true;
        requirements.sampler_anisotropy = true;
        requirements.discrete_gpu = true;
        requirements.device_extensions_names.push(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        vulkan_physical_device_queue_family_info queue_info{};
        bool result = physical_device_meets_requirements(physical_devices[i],surface,
            &curr_props,
            &curr_features,
            &requirements,
            &queue_info,
            &swapchain_support);

        if(result){
            KINFO("Selected device: '%s'.", curr_props.deviceName);
            //GPU type, etc
            switch(curr_props.deviceType){
                default:
                case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                KINFO("GPU type is Unkown.");
                break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                KINFO("GPU type is Integrated.");
                break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                KINFO("GPU type is Virtual.");
                break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                KINFO("GPU type is CPU.");
                break;                
            }

            KINFO("GPU Driver version: %d.%d.%d",
                VK_VERSION_MAJOR(curr_props.driverVersion),
                VK_VERSION_MINOR(curr_props.driverVersion),
                VK_VERSION_PATCH(curr_props.driverVersion));

            //Vulkan API version.
            KINFO("Vulkan API version: %d.%d.%d",
                VK_VERSION_MAJOR(curr_props.apiVersion),
                VK_VERSION_MINOR(curr_props.apiVersion),
                VK_VERSION_PATCH(curr_props.apiVersion));    

            //Memory information
            for(u32 j=0; j < curr_memory.memoryHeapCount; ++j){
                f32 memory_size_gib = (((f32)curr_memory.memoryHeaps[j].size) / 1024.f / 1024.f /1024.f);
                if(curr_memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT){
                    KINFO("Local GPU memory: %.2f GiB", memory_size_gib);
                }
                else 
                {
                    KINFO("Shared System memory: %.2f GiB", memory_size_gib);
                }
                
            }

            physical_device = physical_devices[i];
            graphics_queue_index = queue_info.graphics_family_index;
            present_queue_index = queue_info.present_family_index;
            transfer_queue_index = queue_info.transfer_family_index;

            //keep a copy of properties, features and memory info for later use.
            properties = curr_props;
            features = curr_features;
            memory = curr_memory;
            break;

        }
    }

    //Ensure a device was selected
    if(!physical_device){
        KERROR("No physical devices were found which meet the requirements.");
        return false;
    }

    KINFO("Physical device selected.");
    return true;

}

bool vulkan_device::physical_device_meets_requirements(VkPhysicalDevice device,
        VkSurfaceKHR surface,
        const VkPhysicalDeviceProperties* curr_props,
        const VkPhysicalDeviceFeatures* curr_features,
        const vulkan_physical_device_requirements * requirements,
        vulkan_physical_device_queue_family_info * out_queue_info,
        vulkan_swapchain_support_info*out_swapchain_support){
    //Evaluate device properties to determine if it meets the needs of our application.
    out_queue_info->graphics_family_index = -1;
    out_queue_info->present_family_index = -1;
    out_queue_info->compute_family_index = -1;
    out_queue_info->transfer_family_index = -1;

    //Discrete GPU?
    if(requirements->discrete_gpu){
        if(curr_props->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
            KINFO("Device is not a discrete GPU, and one is required. Skipping.");
            return false;
        }
    }

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device,&queue_family_count, nullptr);
    darray<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device,&queue_family_count,queue_families);

    //Look at each queue and see what queue it support
    KINFO("Graphics | Present | Compute | Transfer | Name");
    u8 min_transfer_score = 255;
    for(u32 i=0; i < queue_family_count; ++i){
        u8 current_transfer_score = 0;

        //Graphics queue?
        if(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
            out_queue_info->graphics_family_index = i;
            ++current_transfer_score;
        }

        //Compute queue
        if(queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT){
            out_queue_info->compute_family_index=i;
            ++current_transfer_score;
        }

        //Transfer queue
        if(queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT){
            //Take the index if it is the current lowest.
            //This increases the chance that it's a dedicated transfer quueeu.
            if(current_transfer_score <= min_transfer_score){
                min_transfer_score = current_transfer_score;
                out_queue_info->transfer_family_index = i;
            }
        }

        //Present queue?
        VkBool32 supports_present = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supports_present));
        if(supports_present){
            out_queue_info->present_family_index = i;
        }
    }

    //Print out some info about the device.
    KINFO("     %d |        %d |        %d |        %d | %s",
        out_queue_info->graphics_family_index != -1,
        out_queue_info->present_family_index != -1,
        out_queue_info->compute_family_index != -1,
        out_queue_info->transfer_family_index != -1,
        curr_props->deviceName);

    if(
        (!requirements->graphics || (requirements->graphics && out_queue_info->graphics_family_index != -1))&&
        (!requirements->present || (requirements->present && out_queue_info->present_family_index != -1))&&
        (!requirements->compute || (requirements->compute && out_queue_info->compute_family_index !=-1))&&
        (!requirements->transfer || (requirements->transfer && out_queue_info->transfer_family_index != -1))){
            KINFO("Device meets queue requirements.");
            KTRACE("Graphics Family Index: %i", out_queue_info->graphics_family_index);
            KTRACE("Present Family Index: %i",out_queue_info->present_family_index);
            KTRACE("Transfer Family Index: %i",out_queue_info->transfer_family_index);
            KTRACE("Compute Family Index: %i",out_queue_info->compute_family_index);

            //query swapchain support
            query_swapchain_support(device, surface, out_swapchain_support);


            if(out_swapchain_support->format_count < 1 || out_swapchain_support->present_mode_count < 1){
                if(out_swapchain_support->formats){
                    kfree(out_swapchain_support->formats, sizeof(VkSurfaceFormatKHR)*out_swapchain_support->format_count, MEMORY_TAG_RENDERER);
                }
                if(out_swapchain_support->present_modes){
                    kfree(out_swapchain_support->present_modes,sizeof(VkPresentModeKHR)*out_swapchain_support->present_mode_count,MEMORY_TAG_RENDERER);
                }
                KINFO("Required swapchain support not present, skipping device.");
                return false;
            }
        
        

    //Device extensions
    if(requirements->device_extensions_names.length()){
        u32 available_extension_count = 0;
        VkExtensionProperties * available_extensions = nullptr;
        VK_CHECK(vkEnumerateDeviceExtensionProperties(
            device,
            nullptr,
            &available_extension_count,
            nullptr));
        if(available_extension_count != 0){
            available_extensions = (VkExtensionProperties*)kallocate(sizeof(VkExtensionProperties)*available_extension_count, MEMORY_TAG_RENDERER);
            VK_CHECK(vkEnumerateDeviceExtensionProperties(
                device,
                nullptr,
                &available_extension_count,
                available_extensions));

                u32 required_extension_count = (u32)requirements->device_extensions_names.length();

                for(u32 i=0; i < required_extension_count; ++i){
                    bool found = false;
                    for(u32 j = 0; j < available_extension_count; ++j){
                        if(strings_equal(requirements->device_extensions_names[i], available_extensions[j].extensionName)){
                            found = true; 
                            break;
                        }
                    }

                    if(!found){
                        KINFO("Required extension not found: '%s', skipping device.",requirements->device_extensions_names[i]);
                        kfree(available_extensions, sizeof(VkExtensionProperties)* available_extension_count, MEMORY_TAG_RENDERER);
                        return false;
                    }
                }
            }
            kfree(available_extensions,sizeof(VkExtensionProperties)*available_extension_count, MEMORY_TAG_RENDERER);
        }
        //sampler anisotropy
        if(requirements->sampler_anisotropy && !curr_features->samplerAnisotropy){
            KINFO("Device does not support samplerAnisotropy, skipping.");
            return false;
        }

        //device meets all reqirements
        return true;
    }
    return false;
}

bool vulkan_device::detect_depth_format(){
    //Format candidates
    const u64 candidate_count = 3;
    VkFormat candidates[3] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    for(u64 i=0; i < candidate_count; ++i){
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physical_device, candidates[i], &props);

        if((props.linearTilingFeatures & flags) == flags){
            depth_format = candidates[i];
            return true;
        }else if((props.optimalTilingFeatures & flags) == flags){
            depth_format = candidates[i];
            return true;
        }
    }
    return true;
}

void vulkan_device::SetResourceName(VkObjectType type, u64 handle, ccharp name)
{

    VkDebugUtilsObjectNameInfoEXT info{VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT};
    info.objectHandle = handle;
    info.objectType = type;
    info.pObjectName = name;
    pfnSetObjectName(logical_device,&info);
}