#pragma once

#include "vulkancore.hpp"
#include "vulkancommands.hpp"

namespace VulkanCube {
    struct CommandPool;

    struct Texture {
        vk::UniqueImage image;
        vk::UniqueDeviceMemory memory;
        vk::UniqueImageView view;
        vk::UniqueSampler sampler;

        static Texture loadFromFile(const Context& ctx, CommandPool& pool, const char* path);
    };
} // namespace VulkanCube