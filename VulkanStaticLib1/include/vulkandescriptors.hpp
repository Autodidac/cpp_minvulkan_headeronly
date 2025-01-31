// vulkandescriptors.hpp
#pragma once
#include "vulkancore.hpp"
#include "vulkanbuffers.hpp"

namespace VulkanCube {
    struct DescriptorSets {
        std::vector<vk::UniqueDescriptorSet> sets;
        vk::UniqueDescriptorPool pool;
        vk::UniqueDescriptorSetLayout layout;

        static DescriptorSets create(const Context& ctx,
            const BufferPackage& uniformBuffer,
            const vk::ImageView& textureImageView);
    };
}