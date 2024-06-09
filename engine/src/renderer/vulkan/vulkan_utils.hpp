#pragma once

#include "vulkan_types.inl"

ccharp vulkan_result_string(VkResult result, bool get_extended);

bool vulkan_result_is_success(VkResult result);