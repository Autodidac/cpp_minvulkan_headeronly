cmake_minimum_required(VERSION 3.20)
project(VulkanStaticLib1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enforce install location at top-level (in the project root)
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/built" CACHE PATH "Install path prefix" FORCE)

# Set shared vcpkg installation directory
if(DEFINED ENV{VCPKG_INSTALLATION_ROOT})
  set(VCPKG_INSTALLATION_ROOT "$ENV{VCPKG_INSTALLATION_ROOT}/vcpkg_installed")
else()
  set(VCPKG_INSTALLATION_ROOT "${CMAKE_SOURCE_DIR}/vcpkg_installed")
endif()

# Find dependencies
find_package(glfw3 REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

# Add subdirectories
#add_subdirectory(VulkanStaticLib1)
#add_subdirectory(VulkanApplication1)
add_subdirectory(vulkanapp1)  # Adding the new VulkanApp1 project
