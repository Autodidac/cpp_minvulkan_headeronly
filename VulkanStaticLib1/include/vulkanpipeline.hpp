#pragma once

#include "../include/vulkanbuffers.hpp"  // Provides the Vertex definition.
#include "../include/vulkancore.hpp"
#include "../include/vulkantextures.hpp"

#include <vector>
#include <array>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace VulkanCube {

    // Reads the entire binary file into a vector of char.
    inline std::vector<char> readFile(std::string_view filename) {
        namespace fs = std::filesystem;
        fs::path filePath{ filename };
        // Use filePath.string() to convert to std::string for std::ifstream.
        std::ifstream file(filePath.string(), std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + std::string(filename));
        }

        const auto fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        return buffer;
    }

    struct GraphicsPipeline {
        vk::UniquePipelineLayout layout;
        vk::UniquePipeline pipeline;
        vk::UniqueRenderPass renderPass;
        vk::UniqueDescriptorSetLayout descriptorSetLayout;

        // Creates the graphics pipeline given the Vulkan context and shader code.
        static GraphicsPipeline createPipeline(
            const Context& ctx,
            const std::vector<char>& vertCode,
            const std::vector<char>& fragCode
        );
    };

    // Creates a unique shader module from the provided shader code.
    inline vk::UniqueShaderModule createShaderModule(vk::Device device, const std::vector<char>& code) {
        if (code.size() % 4 != 0) {
            throw std::runtime_error("Shader code size must be a multiple of 4.");
        }

        vk::ShaderModuleCreateInfo createInfo{
            {}, // flags
            code.size(),
            reinterpret_cast<const uint32_t*>(code.data())
        };

        return device.createShaderModuleUnique(createInfo).value;
    }

} // namespace VulkanCube
