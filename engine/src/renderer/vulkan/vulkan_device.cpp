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


bool vulkan_device::create(VkInstance instance, VkSurfaceKHR surface){
    if(!select_physical_device(instance,surface))
        return false;
    return true;
}

void vulkan_device::destroy(VkInstance instance){

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