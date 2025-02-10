//#include "../pch.h"
#include "../include/vulkantextures.hpp"
#include "../include/vulkanbuffers.hpp"
#include "../include/vulkancommands.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace VulkanCube {

    Texture Texture::loadFromFile(const Context& ctx, CommandPool& pool, const char* path) {
        Texture tex;

        // Load image data
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        vk::DeviceSize imageSize = texWidth * texHeight * 4;

        // Create staging buffer
        BufferPackage staging = BufferPackage::create(
            ctx, imageSize,
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );

        memcpy(staging.mapped, pixels, static_cast<size_t>(imageSize));
        stbi_image_free(pixels);

        // Create image
        vk::ImageCreateInfo imageInfo(
            {}, vk::ImageType::e2D, vk::Format::eR8G8B8A8Srgb,
            { static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1 },
            1, 1, vk::SampleCountFlagBits::e1,
            vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled
        );
        tex.image = ctx.device->createImageUnique(imageInfo).value;

        // Allocate memory
        vk::MemoryRequirements memRequirements = ctx.device->getImageMemoryRequirements(*tex.image);
        vk::MemoryAllocateInfo allocInfo(
            memRequirements.size,
            findMemoryType(ctx.physicalDevice, memRequirements.memoryTypeBits,
                vk::MemoryPropertyFlagBits::eDeviceLocal)
        );
        tex.memory = ctx.device->allocateMemoryUnique(allocInfo).value;
        ctx.device->bindImageMemory(*tex.image, *tex.memory, 0);

        // Transition image layout
        auto cmdBuffer = beginSingleTimeCommands(ctx, pool);

        vk::ImageMemoryBarrier barrier(
            {}, vk::AccessFlagBits::eTransferWrite,
            vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
            *tex.image,
            vk::ImageSubresourceRange(
                vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1
            )
        );
        cmdBuffer->pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe,
            vk::PipelineStageFlagBits::eTransfer,
            {}, {}, {}, barrier
        );

        // Copy buffer to image
        vk::BufferImageCopy region(
            0, 0, 0,
            vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
            { 0, 0, 0 },
            { static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1 }
        );
        cmdBuffer->copyBufferToImage(*staging.buffer, *tex.image,
            vk::ImageLayout::eTransferDstOptimal, 1, &region);

        // Transition to shader read layout
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

        cmdBuffer->pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eFragmentShader,
            {}, {}, {}, barrier
        );

        endSingleTimeCommands(ctx, pool, cmdBuffer.get());

        // Create image view
        vk::ImageViewCreateInfo viewInfo(
            {}, *tex.image, vk::ImageViewType::e2D, vk::Format::eR8G8B8A8Srgb,
            {}, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
        );
        tex.view = ctx.device->createImageViewUnique(viewInfo).value;

        // Create sampler with proper validation and device limits consideration
        vk::SamplerCreateInfo samplerInfo(
            {},
            vk::Filter::eLinear,
            vk::Filter::eLinear,
            vk::SamplerMipmapMode::eLinear,
            vk::SamplerAddressMode::eRepeat,
            vk::SamplerAddressMode::eRepeat,
            vk::SamplerAddressMode::eRepeat,
            0.0f,
            ctx.deviceFeatures.samplerAnisotropy ? VK_TRUE : VK_FALSE,
            ctx.deviceFeatures.samplerAnisotropy ?
            std::min(16.0f, ctx.deviceProperties.limits.maxSamplerAnisotropy) : 1.0f,
            VK_FALSE,
            vk::CompareOp::eAlways,
            0.0f,
            VK_LOD_CLAMP_NONE,
            vk::BorderColor::eIntOpaqueBlack,
            VK_FALSE
        );

        tex.sampler = ctx.device->createSamplerUnique(samplerInfo).value;
        return tex;
    }
} // namespace VulkanCube