# Add executable
add_executable(VulkanApplication1
   VulkanApplication1.cpp
)

# Link VulkanStaticLib1, Vulkan, and GLFW
target_link_libraries(VulkanApplication1 PRIVATE 
    VulkanStaticLib1
    Vulkan::Vulkan
    glfw
)
# Specify include directories for VulkanApplication1
target_include_directories(VulkanApplication1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Use shared vcpkg installation directories
target_include_directories(VulkanApplication1 PRIVATE 
    ${VCPKG_INSTALLATION_ROOT}/x64-linux/include
)
target_link_directories(VulkanApplication1 PRIVATE 
    ${VCPKG_INSTALLATION_ROOT}/x64-linux/lib
)

# If you're targeting X11, make sure that the X11-specific source is added conditionally
if(UNIX AND NOT APPLE)  # Assuming this targets Linux/X11
   # target_sources(VulkanStatic PRIVATE ../../src/VulkanLinux.cpp)
endif()

# Additional linker options for GNU/Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_link_options(VulkanStaticLib1 PRIVATE -Wl,--allow-shlib-undefined)
endif()

# Installation rule for the executable
install(TARGETS VulkanApplication1
    RUNTIME DESTINATION bin
)