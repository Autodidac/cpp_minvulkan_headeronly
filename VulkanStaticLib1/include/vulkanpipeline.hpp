#pragma once

#include "../include/vulkanbuffers.hpp"
#include "../include/vulkancore.hpp"
#include "../include/vulkantextures.hpp"

#include <vector>
#include <array>

namespace VulkanCube {

    std::vector<char> readFile(const std::string& filename);

    struct Vertex {
        glm::vec3 pos;
        glm::vec2 texCoord;

        static vk::VertexInputBindingDescription getBindingDescription();
        static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions();
    };

    struct GraphicsPipeline {
        vk::UniquePipelineLayout layout;
        vk::UniquePipeline pipeline;
        vk::UniqueRenderPass renderPass;
        vk::UniqueDescriptorSetLayout descriptorSetLayout;

        static GraphicsPipeline create(
            const Context& ctx,
            const std::vector<char>& vertCode,
            const std::vector<char>& fragCode
        );
    };

    vk::UniqueShaderModule createShaderModule(vk::Device device, const std::vector<char>& code);
}