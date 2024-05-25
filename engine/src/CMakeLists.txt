cmake_minimum_required(VERSION 3.0.0)
project(KOHICPP VERSION 0.1.0 LANGUAGES C CXX)
file(GLOB SRC_FILES "*.hpp" "*.cpp")
set(SRC_FILES ${COMMON_FILES} PARENT_SCOPE)
add_subdirectory(core)
add_definitions(-DKEXPORT)
add_library(KOHICPP SHARED ${SRC_FILES} ${CORE_FILES})

