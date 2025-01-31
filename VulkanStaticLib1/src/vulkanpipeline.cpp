#include "../pch.h"

#include "../include/vulkanpipeline.hpp"

#include <fstream>

namespace VulkanCube {

    GraphicsPipeline GraphicsPipeline::create(
        const Context& ctx,
        const std::vector<char>& vertCode,
        const std::vector<char>& fragCode
    ) {
        GraphicsPipeline gp;

        // Render pass creation
        std::array<vk::AttachmentDescription, 2> attachments = { {
                // Color attachment
                {
                    {}, ctx.swapchainFormat, vk::SampleCountFlagBits::e1,
                    vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
                    vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                    vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR
                },
            // Depth attachment
            {
                {}, findDepthFormat(ctx.physicalDevice), vk::SampleCountFlagBits::e1,
                vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare,
                vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal
            }
        } };

        vk::AttachmentReference colorRef(0, vk::ImageLayout::eColorAttachmentOptimal);
        vk::AttachmentReference depthRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

        vk::SubpassDescription subpass(
            {}, vk::PipelineBindPoint::eGraphics,
            0, nullptr, 1, &colorRef, nullptr, &depthRef
        );

        vk::SubpassDependency dependency(
            VK_SUBPASS_EXTERNAL, 0,
            vk::PipelineStageFlagBits::eColorAttachmentOutput |
            vk::PipelineStageFlagBits::eEarlyFragmentTests,
            vk::PipelineStageFlagBits::eColorAttachmentOutput |
            vk::PipelineStageFlagBits::eEarlyFragmentTests,
            {},
            vk::AccessFlagBits::eColorAttachmentWrite |
            vk::AccessFlagBits::eDepthStencilAttachmentWrite
        );

        vk::RenderPassCreateInfo renderPassInfo({}, attachments, subpass, dependency);
        gp.renderPass = ctx.device->createRenderPassUnique(renderPassInfo).value;

        // Descriptor set layout
        std::array<vk::DescriptorSetLayoutBinding, 2> bindings = { {
            {0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
            {1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment}
        } };

        gp.descriptorSetLayout = ctx.device->createDescriptorSetLayoutUnique(
            { {}, static_cast<uint32_t>(bindings.size()), bindings.data() }).value;

        // Pipeline layout
        vk::PipelineLayoutCreateInfo layoutInfo({}, 1, &*gp.descriptorSetLayout);
        gp.layout = ctx.device->createPipelineLayoutUnique(layoutInfo).value;

        // Shaders
        auto vertShader = createShaderModule(*ctx.device, vertCode);
        auto fragShader = createShaderModule(*ctx.device, fragCode);

        // Pipeline states
        vk::PipelineShaderStageCreateInfo vertStage(
            {}, vk::ShaderStageFlagBits::eVertex, *vertShader, "main");
        vk::PipelineShaderStageCreateInfo fragStage(
            {}, vk::ShaderStageFlagBits::eFragment, *fragShader, "main");
        std::array stages = { vertStage, fragStage };

        auto bindingDesc = Vertex::getBindingDescription();
        auto attributeDescs = Vertex::getAttributeDescriptions();

        vk::PipelineVertexInputStateCreateInfo vertexInput(
            {}, 1, &bindingDesc,
            static_cast<uint32_t>(attributeDescs.size()), attributeDescs.data());

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
            {}, vk::PrimitiveTopology::eTriangleList);

        vk::Viewport viewport(0.0f, 0.0f,
            static_cast<float>(ctx.swapchainExtent.width),
            static_cast<float>(ctx.swapchainExtent.height),
            0.0f, 1.0f);
        vk::Rect2D scissor({ 0, 0 }, ctx.swapchainExtent);
        vk::PipelineViewportStateCreateInfo viewportState({}, 1, &viewport, 1, &scissor);

        vk::PipelineRasterizationStateCreateInfo rasterizer(
            {}, false, false, vk::PolygonMode::eFill,
            vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise);

        vk::PipelineMultisampleStateCreateInfo multisampling;
        vk::PipelineDepthStencilStateCreateInfo depthStencil(
            {}, true, true, vk::CompareOp::eLess);

        vk::PipelineColorBlendAttachmentState colorBlendAttachment;
        colorBlendAttachment.colorWriteMask =
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

        vk::PipelineColorBlendStateCreateInfo colorBlending(
            {}, false, vk::LogicOp::eCopy, 1, &colorBlendAttachment);

        // Pipeline creation
        vk::GraphicsPipelineCreateInfo pipelineInfo(
            {}, stages, &vertexInput, &inputAssembly,
            nullptr, &viewportState, &rasterizer, &multisampling,
            &depthStencil, &colorBlending, nullptr,
            *gp.layout, *gp.renderPass
        );

        gp.pipeline = ctx.device->createGraphicsPipelineUnique(nullptr, pipelineInfo).value;

        return gp;
    }

    vk::VertexInputBindingDescription VulkanCube::Vertex::getBindingDescription() {
        return { 0, sizeof(VulkanCube::Vertex), vk::VertexInputRate::eVertex };
    }

    std::array<vk::VertexInputAttributeDescription, 2> VulkanCube::Vertex::getAttributeDescriptions() {
        return { {
            {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(VulkanCube::Vertex, pos)},
            {1, 0, vk::Format::eR32G32Sfloat, offsetof(VulkanCube::Vertex, texCoord)}
        } };
    }

    std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    vk::UniqueShaderModule createShaderModule(vk::Device device, const std::vector<char>& code) {
        vk::ShaderModuleCreateInfo createInfo(
            {}, code.size(), reinterpret_cast<const uint32_t*>(code.data()));
        return device.createShaderModuleUnique(createInfo).value;
    }
} // namespace VulkanCube