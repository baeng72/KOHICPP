cmake_minimum_required(VERSION 3.0.0)
project(KOHICPP VERSION 0.1.0 LANGUAGES C CXX)
if(WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -DVK_USE_PLATFORM_WIN32_KHR -DNOMINMAX -DKPLATFORM_GLFW)
endif()

add_subdirectory(memory)

file(GLOB TESTS_FILES "*.cpp" "*.hpp")
set(TESTS_FILES ${TESTS_FILES} PARENT_SCOPE)
add_executable(TESTS ${TESTS_FILES} ${MEMORY_TEST_FILES})



target_link_libraries(TESTS KOHICPP)
