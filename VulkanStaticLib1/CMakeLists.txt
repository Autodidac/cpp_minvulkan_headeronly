cmake_minimum_required(VERSION 3.20)
project(vulkan_cube)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

add_subdirectory(example)