// vulkandescriptors.hpp
#pragma once

#include <vector>
#include "vulkancore.hpp"
#include "vulkanbuffers.hpp"

namespace VulkanCube {

    struct DescriptorSets {
        std::vector<vk::DescriptorSet> sets; // Changed from UniqueDescriptorSet to DescriptorSet.
        vk::UniqueDescriptorPool pool;
        vk::UniqueDescriptorSetLayout layout;

        static DescriptorSets create(const Context& ctx,
                                     const BufferPackage& uniformBuffer,
                                     const vk::ImageView& textureImageView);
    };
} // namespace VulkanCube
