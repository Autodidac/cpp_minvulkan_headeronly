#include "..\pch.h"
#include "..\include\vulkanbuffers.hpp"

namespace VulkanCube {
   // using namespace vk;

    std::array<vk::VertexInputAttributeDescription, 2> Vertex::getAttributeDescriptions() {
        return { {
            {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)},
            {1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord)}
        } };
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