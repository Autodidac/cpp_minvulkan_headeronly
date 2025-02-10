# Create executable for vulkanapp1
add_executable(vulkanapp1
    vulkanapp1.cpp
)

# Find Vulkan
find_package(glfw3 REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

# Link VulkanStaticLib1 and Vulkan
target_link_libraries(vulkanapp1 PRIVATE 
    Vulkan::Vulkan  # <-- This was missing
    glfw            # <-- Add this if GLFW is used
    glm::glm
)

# Include directories for vulkanapp1
target_include_directories(vulkanapp1 PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${VCPKG_INSTALLATION_ROOT}/x64-linux/include
)

# Link directories for VulkanApp1
target_link_directories(vulkanapp1 PRIVATE 
    ${VCPKG_INSTALLATION_ROOT}/x64-linux/lib
)

# Installation rule for vulkanapp1 executable
install(TARGETS vulkanapp1
    RUNTIME DESTINATION bin
)

# Fix missing symbols for GNU/Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_link_options(vulkanapp1 PRIVATE -Wl,--allow-shlib-undefined)
endif()
