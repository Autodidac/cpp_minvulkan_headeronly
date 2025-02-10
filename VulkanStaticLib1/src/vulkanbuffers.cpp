//#include "..\pch.h"
#include "../include/vulkanbuffers.hpp"

namespace VulkanCube {
   // using namespace vk;
std::array<vk::VertexInputAttributeDescription, 3> VulkanCube::Vertex::getAttributeDescriptions() {
    std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions{};

    // Position
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    // Color
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    // Texture Coordinates
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
}

    BufferPackage BufferPackage::create(const Context& ctx, vk::DeviceSize size,
        vk::BufferUsageFlags usage,
        vk::MemoryPropertyFlags properties) {
        BufferPackage bp;

        // Buffer creation
        vk::BufferCreateInfo bufferInfo({}, size, usage);
        bp.buffer = ctx.device->createBufferUnique(bufferInfo).value;

        // Memory allocation
        vk::MemoryRequirements memReq = ctx.device->getBufferMemoryRequirements(*bp.buffer);
        vk::MemoryAllocateInfo allocInfo(memReq.size,
            VulkanCube::findMemoryType(ctx.physicalDevice, memReq.memoryTypeBits, properties));

        bp.memory = ctx.device->allocateMemoryUnique(allocInfo).value;
        
        // Handle the result:
        vk::Result result = ctx.device->bindBufferMemory(*bp.buffer, *bp.memory, 0);
        if (result != vk::Result::eSuccess) {
            // Handle error (e.g., throw or log)
        }

        if (properties & vk::MemoryPropertyFlagBits::eHostVisible) {
            bp.mapped = ctx.device->mapMemory(*bp.memory, 0, size).value;
        }

        return bp;
    }
} // namespace VulkanCube