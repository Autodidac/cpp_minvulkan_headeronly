#pragma once

#include "vulkanpipeline.hpp"
#include "vulkancore.hpp"
#include "vulkanbuffers.hpp"

namespace VulkanCube {
    // Forward declarations
    struct Context;             // Declared in vulkancore.hpp
    struct GraphicsPipeline;    // Declared in vulkanpipeline.hpp
    struct BufferPackage;       // Declared in vulkanbuffers.hpp

    struct CommandPool {
        vk::UniqueCommandPool pool;
        std::vector<vk::UniqueCommandBuffer> buffers;

        static CommandPool create(const Context& ctx, uint32_t bufferCount);

        void record(
            const Context& ctx,
            const GraphicsPipeline& pipeline,
            const BufferPackage& vertexBuffer,
            const BufferPackage& indexBuffer,
            const std::vector<vk::UniqueFramebuffer>& framebuffers,
            const std::vector<vk::UniqueDescriptorSet>& descriptorSets,
            const std::vector<uint16_t>& indices
        ) const;
    };

    vk::UniqueCommandBuffer beginSingleTimeCommands(const Context& ctx, const CommandPool& pool);
    void endSingleTimeCommands(const Context& ctx, const CommandPool& pool, vk::CommandBuffer commandBuffer);
}