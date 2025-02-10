//#include "../pch.h"
#include "../include/vulkanpipeline.hpp"

// If you're on a non-MSVC compiler, add a fallback for strncpy_s to avoid errors in Vulkan headers.
#ifndef _MSC_VER
  #define strncpy_s(dest, destsz, src, count) strncpy(dest, src, count)
#endif

namespace VulkanCube {

    GraphicsPipeline GraphicsPipeline::createPipeline(
        const Context& ctx,
        const std::vector<char>& vertCode,
        const std::vector<char>& fragCode
    ) {
        GraphicsPipeline gp;

        // Render pass creation
        std::array<vk::AttachmentDescription, 2> attachments = { {
            // Color attachment
            {
                {},
                ctx.swapchainFormat,
                vk::SampleCountFlagBits::e1,
                vk::AttachmentLoadOp::eClear,
                vk::AttachmentStoreOp::eStore,
                vk::AttachmentLoadOp::eDontCare,
                vk::AttachmentStoreOp::eDontCare,
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::ePresentSrcKHR
            },
            // Depth attachment
            {
                {},
                findDepthFormat(ctx.physicalDevice),
                vk::SampleCountFlagBits::e1,
                vk::AttachmentLoadOp::eClear,
                vk::AttachmentStoreOp::eDontCare,
                vk::AttachmentLoadOp::eDontCare,
                vk::AttachmentStoreOp::eDontCare,
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eDepthStencilAttachmentOptimal
            }
        } };

        vk::AttachmentReference colorRef(0, vk::ImageLayout::eColorAttachmentOptimal);
        vk::AttachmentReference depthRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

        vk::SubpassDescription subpass(
            {},
            vk::PipelineBindPoint::eGraphics,
            0, nullptr,
            1, &colorRef,
            nullptr, &depthRef
        );

        vk::SubpassDependency dependency(
            VK_SUBPASS_EXTERNAL, 0,
            vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
            vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
            {},
            vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite
        );

        vk::RenderPassCreateInfo renderPassInfo({}, attachments, subpass, dependency);
        gp.renderPass = ctx.device->createRenderPassUnique(renderPassInfo).value;

        // Descriptor set layout
        std::array<vk::DescriptorSetLayoutBinding, 2> bindings = { {
            {0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
            {1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment}
        } };

        gp.descriptorSetLayout = ctx.device->createDescriptorSetLayoutUnique(
            { {}, static_cast<uint32_t>(bindings.size()), bindings.data() }
        ).value;

        // Pipeline layout
        vk::PipelineLayoutCreateInfo layoutInfo({}, 1, &*gp.descriptorSetLayout);
        gp.layout = ctx.device->createPipelineLayoutUnique(layoutInfo).value;

        // Shaders (using inline createShaderModule from the header)
        auto vertShader = createShaderModule(*ctx.device, vertCode);
        auto fragShader = createShaderModule(*ctx.device, fragCode);

        // Pipeline stages
        vk::PipelineShaderStageCreateInfo vertStage(
            {},
            vk::ShaderStageFlagBits::eVertex,
            *vertShader,
            "main"
        );
        vk::PipelineShaderStageCreateInfo fragStage(
            {},
            vk::ShaderStageFlagBits::eFragment,
            *fragShader,
            "main"
        );
        std::array stages = { vertStage, fragStage };

        // Using the Vertex definition from vulkanbuffers.hpp.
        auto bindingDesc = Vertex::getBindingDescription();
        auto attributeDescs = Vertex::getAttributeDescriptions();

        vk::PipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.flags = vk::PipelineVertexInputStateCreateFlags{};
        vertexInput.vertexBindingDescriptionCount = 1;
        vertexInput.pVertexBindingDescriptions = reinterpret_cast<const vk::VertexInputBindingDescription*>(&bindingDesc);
        vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescs.size());
        vertexInput.pVertexAttributeDescriptions = attributeDescs.data();
        vertexInput.pNext = nullptr;

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
            {},
            vk::PrimitiveTopology::eTriangleList
        );

        vk::Viewport viewport(
            0.0f, 0.0f,
            static_cast<float>(ctx.swapchainExtent.width),
            static_cast<float>(ctx.swapchainExtent.height),
            0.0f, 1.0f
        );
        vk::Rect2D scissor({ 0, 0 }, ctx.swapchainExtent);
        vk::PipelineViewportStateCreateInfo viewportState({}, 1, &viewport, 1, &scissor);

        vk::PipelineRasterizationStateCreateInfo rasterizer(
            {},
            false, false,
            vk::PolygonMode::eFill,
            vk::CullModeFlagBits::eBack,
            vk::FrontFace::eCounterClockwise
        );

        vk::PipelineMultisampleStateCreateInfo multisampling;
        vk::PipelineDepthStencilStateCreateInfo depthStencil(
            {},
            true, true,
            vk::CompareOp::eLess
        );

        vk::PipelineColorBlendAttachmentState colorBlendAttachment;
        colorBlendAttachment.colorWriteMask =
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

        vk::PipelineColorBlendStateCreateInfo colorBlending(
            {},
            false,
            vk::LogicOp::eCopy,
            1, &colorBlendAttachment
        );

        // Pipeline creation with subpass index 0 explicitly specified.
        vk::GraphicsPipelineCreateInfo pipelineInfo(
            {},
            stages,
            &vertexInput,
            &inputAssembly,
            nullptr,
            &viewportState,
            &rasterizer,
            &multisampling,
            &depthStencil,
            &colorBlending,
            nullptr,
            *gp.layout,
            *gp.renderPass,
            0  // Subpass index
        );

        gp.pipeline = ctx.device->createGraphicsPipelineUnique(nullptr, pipelineInfo).value;
        return gp;
    }

} // namespace VulkanCube
