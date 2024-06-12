cmake_minimum_required(VERSION 3.0.0)
project(KOHICPP VERSION 0.1.0 LANGUAGES C CXX)

find_package(Vulkan)
include_directories(AFTER ${Vulkan_INCLUDE_DIRS})
#include_directories(Vulkan_INCLUDE_DIRS)

include_directories(engine/src)
add_subdirectory(engine/src)
add_subdirectory(testbed/src)
add_subdirectory(tests/src)




