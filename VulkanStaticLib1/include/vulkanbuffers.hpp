#pragma once

#include "vulkancore.hpp"

#include <glm/glm.hpp>

namespace VulkanCube {
    struct Vertex {
        glm::vec3 pos;
        glm::vec2 texCoord;

        static inline vk::VertexInputBindingDescription getBindingDescription() {
            return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex };
        }

        static inline std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions() {
            return { {
                {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)},
                {1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord)}
            } };
        }
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
}