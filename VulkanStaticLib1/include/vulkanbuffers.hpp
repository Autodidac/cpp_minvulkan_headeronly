#pragma once

#include "vulkancore.hpp"

#include <glm/glm.hpp>

namespace VulkanCube {

struct Vertex {
    alignas(16) float pos[3];      // Position (x, y, z)
    alignas(16) float color[3];    // Color (r, g, b)
    alignas(8)  float texCoord[2];  // Texture coordinates (u, v)

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions();

};

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct BufferPackage {
    vk::UniqueBuffer buffer;
    vk::UniqueDeviceMemory memory;
    void* mapped = nullptr;

    static BufferPackage create(const Context& ctx, vk::DeviceSize size,
                                vk::BufferUsageFlags usage,
                                vk::MemoryPropertyFlags properties);
};

} // namespace VulkanCube
