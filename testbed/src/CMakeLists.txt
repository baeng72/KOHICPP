cmake_minimum_required(VERSION 3.0.0)
project(KOHICPP VERSION 0.1.0 LANGUAGES C CXX)
if(WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -DVK_USE_PLATFORM_WIN32_KHR -DNOMINMAX)
endif()
add_executable(TESTBED main.cpp)

target_link_libraries(TESTBED KOHICPP)
