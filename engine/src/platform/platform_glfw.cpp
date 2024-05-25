#include "platform/platform.hpp"
#include "core/logger.hpp"
#include <cstdlib>
#if defined(KPLATFORM_WINDOWS)
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

// Clock
static f64 clock_frequency;
static LARGE_INTEGER start_time;

#endif

#if defined(KPLATFORM_GLFW)

#include <glfw/glfw3.h>

bool platform::startup(ccharp application_name, i32 x, i32 y, i32 width, i32 height){
    if(!glfwInit()){
        KFATAL("Unable to initialized GLFW!");
        return false;
    }

    if(!glfwVulkanSupported()){
        KFATAL("GLFW doesn't support Vulkan!");
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow * pwindow = glfwCreateWindow(width,height,application_name,nullptr,nullptr);
    glfwSetWindowPos(pwindow,x,y);//position window

    glfwSetWindowUserPointer(pwindow,this);

    glfwSetWindowSizeCallback(pwindow,[](GLFWwindow*pwindow, i32 width, i32 height){
        platform*pplatform = (platform*)glfwGetWindowUserPointer(pwindow);
    });

    glfwSetWindowCloseCallback(pwindow,[](GLFWwindow*pwindow){
        platform*pplatform = (platform*)glfwGetWindowUserPointer(pwindow);
    });

    glfwSetKeyCallback(pwindow,[](GLFWwindow*pwindow,i32 key, i32 scancode, i32 action, i32 mods){
        platform*pplatform = (platform*)glfwGetWindowUserPointer(pwindow);

    });
    glfwSetMouseButtonCallback(pwindow,[](GLFWwindow*pwindow,i32 buttion, i32 action, i32 mods){
        platform*pplatform = (platform*)glfwGetWindowUserPointer(pwindow);
    });
    glfwSetScrollCallback(pwindow,[](GLFWwindow*pwindow, f64 xoffset, f64 yoffset){
            platform*pplatform = (platform*)glfwGetWindowUserPointer(pwindow);
    });
    glfwSetCursorPosCallback(pwindow,[](GLFWwindow*pwindow,f64 xpos, f64 ypos){
        platform*pplatform = (platform*)glfwGetWindowUserPointer(pwindow);
    });
    glfwSetCharCallback(pwindow,[](GLFWwindow*pwindow, u32 keycode){
        platform*pplatform = (platform*)glfwGetWindowUserPointer(pwindow);
    });

#if defined(KPLATFORM_WINDOWS)
    // Clock setup
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clock_frequency = 1.0 / (f64)frequency.QuadPart;
    QueryPerformanceCounter(&start_time);
#endif    


    internal_state = pwindow;
    return true;
}

void platform::shutdown(){
    GLFWwindow*pwindow = static_cast<GLFWwindow*>(internal_state);
    glfwDestroyWindow(pwindow);
    glfwTerminate();
}

bool platform::pump_messages(){
    GLFWwindow*pwindow = static_cast<GLFWwindow*>(internal_state);
    while(!glfwWindowShouldClose(pwindow)){
        glfwPollEvents();
    }
    return true;
}

void * platform_allocate(u64 size, bool aligned){
    return malloc(size);
}

void platform_free(void*block, bool aligned){
    free(block);
}

void *platform_zero_memory(void* block, u64 size){
    return memset(block, 0, size);
}

void * platform_copy_memory(void* dest, const void* source, u64 size){
    return memcpy(dest, source, size);
}

void * platform_set_memory(void*dest, i32 value, u64 size){
    return memset(dest, value, size);
}

void platform_console_write(ccharp message, u8 color){
#if defined(KPLATFORM_WINDOWS)
        HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[color]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, number_written, 0);
#endif    
}

void platform_console_write_error(ccharp message, u8 color){
#if defined(KPLATFORM_WINDOWS)
    HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);
    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[color]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, number_written, 0);
#endif
}

f64 platform_get_absolute_time(){
#if defined(KPLATFORM_WINDOWS)
    LARGE_INTEGER now_time;
    QueryPerformanceCounter(&now_time);
    return (f64)now_time.QuadPart * clock_frequency;
#else
    return glfwGetTime();
#endif
}




#endif