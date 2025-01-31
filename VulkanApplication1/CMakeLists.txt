add_library(vulkan_cube STATIC 
    ../src/vulkancore.cpp
    ../src/vulkanbuffers.cpp
    ../src/vulkantextures.cpp
    ../src/vulkanpipeline.cpp
    ../src/vulkancommands.cpp)

target_include_directories(vulkan_cube PUBLIC ../include)
target_link_libraries(vulkan_cube Vulkan::Vulkan glfw)

add_executable(cube_example VulkanApplication1.cpp)
target_link_libraries(cube_example vulkan_cube)