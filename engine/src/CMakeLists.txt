cmake_minimum_required(VERSION 3.0.0)
project(KOHICPP VERSION 0.1.0 LANGUAGES C CXX)
file(GLOB SRC_FILES "*.hpp" "*.cpp")
set(SRC_FILES ${SRC_FILES} PARENT_SCOPE)

# GLFW options
set(GLFW_VULKAN_STATIC ON CACHE BOOL "")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")
include_directories(vendor/glfw/include)

find_package(Vulkan)
include_directories(AFTER ${Vulkan_INCLUDE_DIRS})

link_libraries(${Vulkan_LIBRARY})

add_subdirectory(vendor)
add_subdirectory(core)
add_subdirectory(platform)
add_subdirectory(renderer)
add_subdirectory(math)
add_definitions(-DKEXPORT)

add_library(KOHICPP SHARED ${SRC_FILES} ${CORE_FILES} ${PLATFORM_FILES} ${CONTAINER_FILES} ${RENDERER_FILES} ${MATH_FILES})

target_link_libraries(KOHICPP glfw)

