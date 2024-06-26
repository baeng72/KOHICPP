#include "platform/platform.hpp"
#include "core/logger.hpp"
#include "core/input.hpp"
#include "core/event.hpp"
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
#define GLFW_INCLUDE_VULKAN

#include <glfw/glfw3.h>

#include "renderer/vulkan/vulkan_types.inl"

platform_system*platform_system::state_ptr{nullptr};

bool platform_system::startup(ccharp application_name, i32 x, i32 y, i32 width, i32 height){
    if(state_ptr==nullptr){
        state_ptr = this;
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
            //platform_system*pplatform = (platform_system*)glfwGetWindowUserPointer(pwindow);
            event_context context;
            context.u16[0] = (u16)width;
            context.u16[1] = (u16)height;
            event_fire(EVENT_CODE_RESIZED,0,context);
        });

        glfwSetWindowCloseCallback(pwindow,[](GLFWwindow*pwindow){
            //platform_system*pplatform = (platform_system*)glfwGetWindowUserPointer(pwindow);
            event_context data{};
            event_fire(EVENT_CODE_APPLICATION_QUIT,0,data);
        });

        glfwSetKeyCallback(pwindow,[](GLFWwindow*pwindow,i32 key, i32 scancode, i32 action, i32 mods){
            //platform_system*pplatform = (platform_system*)glfwGetWindowUserPointer(pwindow);
            
            keys key_val = (keys)key;
            bool pressed = action == GLFW_PRESS;
            input_process_key(key_val, pressed);

        });
        glfwSetMouseButtonCallback(pwindow,[](GLFWwindow*pwindow,i32 button, i32 action, i32 mods){
            //platform_system*pplatform = (platform_system*)glfwGetWindowUserPointer(pwindow);
            buttons button_val = (buttons)button;
            bool pressed = action == GLFW_PRESS;
            input_process_button(button_val,pressed);
        });
        glfwSetScrollCallback(pwindow,[](GLFWwindow*pwindow, f64 xoffset, f64 yoffset){
            //platform_system*pplatform = (platform_system*)glfwGetWindowUserPointer(pwindow);
            if(yoffset != 0.0){
            i8 z_delta = yoffset <0 ? -1 : 1;
                input_process_mouse_wheel(z_delta);
            }
        });
        glfwSetCursorPosCallback(pwindow,[](GLFWwindow*pwindow,f64 xpos, f64 ypos){
            //platform_system*pplatform = (platform_system*)glfwGetWindowUserPointer(pwindow);
            input_process_mouse_move((i16)xpos,(i16)ypos);
        });
        glfwSetCharCallback(pwindow,[](GLFWwindow*pwindow, u32 keycode){
            //platform_system*pplatform = (platform_system*)glfwGetWindowUserPointer(pwindow);
        });

    #if defined(KPLATFORM_WINDOWS)
        // Clock setup
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        clock_frequency = 1.0 / (f64)frequency.QuadPart;
        QueryPerformanceCounter(&start_time);
    #endif    


        state_ptr->internal_state = pwindow;
        return true;
    }
    return false;
}

void platform_system::shutdown(){
    GLFWwindow*pwindow = static_cast<GLFWwindow*>(state_ptr->internal_state);
    glfwDestroyWindow(pwindow);
    glfwTerminate();
}

bool platform_system::pump_messages(){
    if(state_ptr==nullptr)
        return false;

    GLFWwindow*pwindow = static_cast<GLFWwindow*>(state_ptr->internal_state);
    if(glfwWindowShouldClose(pwindow))
        return false;
    glfwPollEvents();
    
    return true;
}

void platform_system::get_required_extensions_names(darray<ccharp>&names_array){
    u32 count = 0;
    auto ext = glfwGetRequiredInstanceExtensions(&count);
    for(u32 i=0;i<count;i++){
        names_array.push(ext[i]);
    }
}

bool platform_system::create_vulkan_surface(vulkan_context*context){
    if(state_ptr){
        GLFWwindow*pwindow = static_cast<GLFWwindow*>(state_ptr->internal_state);
        VkResult res = glfwCreateWindowSurface(context->instance,   pwindow, nullptr, &context->surface);
        return res==VK_SUCCESS;

    }
    return false;
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

void platform_sleep(u64 ms){
    Sleep((DWORD)ms);
}


#endif