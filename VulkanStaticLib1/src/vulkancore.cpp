//#include "..\pch.h"
#include "../include/vulkancore.hpp"

#include <set>
#include <stdexcept>
#include <limits>
#include <algorithm>

namespace VulkanCube {
    //using namespace vk;

    const std::vector<const char*> Context::deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    // Helper implementations...
    bool QueueFamilyIndices::isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }

    SwapChainSupportDetails Context::querySwapChainSupport() const {
        SwapChainSupportDetails details;
        details.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface).value;
        details.formats = physicalDevice.getSurfaceFormatsKHR(*surface).value;
        details.presentModes = physicalDevice.getSurfacePresentModesKHR(*surface).value;
        return details;
    }

    // Helper functions
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats) {
        for (const auto& format : formats) {
            if (format.format == vk::Format::eB8G8R8A8Srgb &&
                format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return format;
            }
        }
        return formats[0];
    }

    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& presentModes) {
        for (const auto& mode : presentModes) {
            if (mode == vk::PresentModeKHR::eMailbox) return mode;
        }
        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        return vk::Extent2D{
            std::clamp(static_cast<uint32_t>(width),
                       capabilities.minImageExtent.width,
                       capabilities.maxImageExtent.width),
            std::clamp(static_cast<uint32_t>(height),
                       capabilities.minImageExtent.height,
                       capabilities.maxImageExtent.height)
        };
    }

    Context Context::createWindow(GLFWwindow* window, bool enableValidation) {
        Context ctx;

        // Instance creation
        vk::ApplicationInfo appInfo("Vulkan Cube", VK_MAKE_VERSION(1, 0, 0),
            "No Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_3);

        std::vector<const char*> extensions;
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        extensions.assign(glfwExtensions, glfwExtensions + glfwExtensionCount);

        vk::InstanceCreateInfo createInfo({}, &appInfo, 0, nullptr,
            static_cast<uint32_t>(extensions.size()), extensions.data());

        ctx.instance = createInstanceUnique(createInfo).value;

        // Surface creation
        VkSurfaceKHR surface;
        if (glfwCreateWindowSurface(*ctx.instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface!");
        }
        ctx.surface = vk::UniqueSurfaceKHR(surface, *ctx.instance);

        // Physical device selection
        auto devices = ctx.instance->enumeratePhysicalDevices().value;
        for (const auto& device : devices) {
            QueueFamilyIndices indices;
            auto queueFamilies = device.getQueueFamilyProperties();

            int i = 0;
            for (const auto& queueFamily : queueFamilies) {
                if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                    indices.graphicsFamily = i;
                }

                if (device.getSurfaceSupportKHR(i, *ctx.surface).value) {
                    indices.presentFamily = i;
                }

                if (indices.isComplete()) break;
                i++;
            }

            if (indices.isComplete()) {
                ctx.physicalDevice = device;
                ctx.queueIndices = indices;
                break;
            }
        }

        // Device creation
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {
            ctx.queueIndices.graphicsFamily.value(),
            ctx.queueIndices.presentFamily.value()
        };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            queueCreateInfos.push_back({ {}, queueFamily, 1, &queuePriority });
        }

        vk::PhysicalDeviceFeatures deviceFeatures;
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        vk::DeviceCreateInfo deviceInfo({},
            static_cast<uint32_t>(queueCreateInfos.size()), queueCreateInfos.data(),
            0, nullptr,
            static_cast<uint32_t>(deviceExtensions.size()), deviceExtensions.data(),
            &deviceFeatures);

        ctx.device = ctx.physicalDevice.createDeviceUnique(deviceInfo).value;
        ctx.graphicsQueue = ctx.device->getQueue(ctx.queueIndices.graphicsFamily.value(), 0);
        ctx.presentQueue = ctx.device->getQueue(ctx.queueIndices.presentFamily.value(), 0);
        // store some values
        ctx.deviceFeatures = ctx.physicalDevice.getFeatures();
        ctx.deviceProperties = ctx.physicalDevice.getProperties();

        // Swapchain creation
        SwapChainSupportDetails swapChainSupport = ctx.querySwapChainSupport();
        auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        auto presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        ctx.swapchainExtent = chooseSwapExtent(swapChainSupport.capabilities, window);
        ctx.swapchainFormat = surfaceFormat.format;

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR swapchainInfo(
            {}, *ctx.surface, imageCount,
            ctx.swapchainFormat, surfaceFormat.colorSpace,
            ctx.swapchainExtent, 1,
            vk::ImageUsageFlagBits::eColorAttachment,
            ctx.queueIndices.graphicsFamily == ctx.queueIndices.presentFamily ?
            vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent,
            {}, swapChainSupport.capabilities.currentTransform,
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            presentMode, VK_TRUE, nullptr
        );

        if (ctx.queueIndices.graphicsFamily != ctx.queueIndices.presentFamily) {
            std::vector<uint32_t> queueFamilyIndices = {
                ctx.queueIndices.graphicsFamily.value(),
                ctx.queueIndices.presentFamily.value()
            };
            swapchainInfo.queueFamilyIndexCount = 2;
            swapchainInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        }

        ctx.swapchain = ctx.device->createSwapchainKHRUnique(swapchainInfo).value;
        ctx.swapchainImages = ctx.device->getSwapchainImagesKHR(*ctx.swapchain).value;

        // Create image views for the swapchain images
        ctx.swapchainImageViews.resize(ctx.swapchainImages.size());
        for (size_t i = 0; i < ctx.swapchainImages.size(); i++) {
            vk::ImageViewCreateInfo createInfo(
                {}, ctx.swapchainImages[i], vk::ImageViewType::e2D,
                ctx.swapchainFormat, {},
                { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
            );
            ctx.swapchainImageViews[i] = ctx.device->createImageViewUnique(createInfo).value;
        }

        // Depth resources
        vk::Format depthFormat = findDepthFormat(ctx.physicalDevice);
        vk::ImageCreateInfo depthImageInfo(
            {}, vk::ImageType::e2D, depthFormat,
            { ctx.swapchainExtent.width, ctx.swapchainExtent.height, 1 },
            1, 1, vk::SampleCountFlagBits::e1,
            vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eDepthStencilAttachment,
            vk::SharingMode::eExclusive
        );

        ctx.depthImage = ctx.device->createImageUnique(depthImageInfo).value;
        vk::MemoryRequirements memReq = ctx.device->getImageMemoryRequirements(*ctx.depthImage);
        vk::MemoryAllocateInfo allocInfo(
            memReq.size,
            findMemoryType(ctx.physicalDevice, memReq.memoryTypeBits,
                vk::MemoryPropertyFlagBits::eDeviceLocal)
        );
        ctx.depthImageMemory = ctx.device->allocateMemoryUnique(allocInfo).value;

        vk::Result result = ctx.device->bindImageMemory(*ctx.depthImage, *ctx.depthImageMemory, 0);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to bind depth image memory!");
        }

        vk::ImageViewCreateInfo depthViewInfo(
            {}, *ctx.depthImage, vk::ImageViewType::e2D, depthFormat, {},
            { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 }
        );
        ctx.depthImageView = ctx.device->createImageViewUnique(depthViewInfo).value;

        // **** New Code: Create a default Render Pass ****
        vk::AttachmentDescription colorAttachment(
            {}, ctx.swapchainFormat, vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR
        );
        vk::AttachmentDescription depthAttachment(
            {}, depthFormat, vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare,
            vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal
        );
        std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
        vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);
        vk::AttachmentReference depthAttachmentRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
        vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, 0, nullptr,
                                       1, &colorAttachmentRef, nullptr, &depthAttachmentRef);
vk::RenderPassCreateInfo renderPassInfo(
    {},
    static_cast<uint32_t>(attachments.size()),  // attachment count (2)
    attachments.data(),                         // pointer to attachments
    1,                                          // subpass count
    &subpass
);

        ctx.renderPass = ctx.device->createRenderPassUnique(renderPassInfo).value;
        // **************************************************

        // **** New Code: Create Framebuffers for each swapchain image view ****
        ctx.swapchainFramebuffers.resize(ctx.swapchainImageViews.size());
        for (size_t i = 0; i < ctx.swapchainImageViews.size(); i++) {
            std::array<vk::ImageView, 2> fbAttachments = {
                *ctx.swapchainImageViews[i],
                *ctx.depthImageView
            };
            vk::FramebufferCreateInfo framebufferInfo(
                {}, *ctx.renderPass, static_cast<uint32_t>(fbAttachments.size()),
                fbAttachments.data(), ctx.swapchainExtent.width, ctx.swapchainExtent.height, 1
            );
            ctx.swapchainFramebuffers[i] = ctx.device->createFramebufferUnique(framebufferInfo).value;
        }
        // *************************************************************************

        // (Optionally, create synchronization objects here or via createSyncObjects())
        ctx.createSyncObjects();

        return ctx;
    }

    void Context::createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            imageAvailableSemaphores[i] = device->createSemaphoreUnique({}).value;
            renderFinishedSemaphores[i] = device->createSemaphoreUnique({}).value;
            inFlightFences[i] = device->createFenceUnique({ vk::FenceCreateFlagBits::eSignaled }).value;
        }
    }

    void Context::recreateSwapchain(GLFWwindow* window) {
        device->waitIdle();

        swapchainImageViews.clear();
        swapchainFramebuffers.clear();

        // Recreate swapchain
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport();
        vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        vk::Extent2D newExtent = chooseSwapExtent(swapChainSupport.capabilities, window);
          vk::UniqueImageView depthImageView; // Ensure this exists

        vk::SwapchainCreateInfoKHR createInfo(
            {}, *surface, swapChainSupport.capabilities.minImageCount + 1,
            surfaceFormat.format, surfaceFormat.colorSpace,
            newExtent, 1, vk::ImageUsageFlagBits::eColorAttachment,
            vk::SharingMode::eExclusive, {},
            swapChainSupport.capabilities.currentTransform,
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            chooseSwapPresentMode(swapChainSupport.presentModes),
            VK_TRUE
        );

        swapchain = device->createSwapchainKHRUnique(createInfo).value;
        swapchainImages = device->getSwapchainImagesKHR(*swapchain).value;
        swapchainFormat = surfaceFormat.format;
        swapchainExtent = newExtent;

        // Recreate image views
        swapchainImageViews.resize(swapchainImages.size());
        for (size_t i = 0; i < swapchainImages.size(); i++) {
            vk::ImageViewCreateInfo viewInfo(
                {}, swapchainImages[i], vk::ImageViewType::e2D,
                swapchainFormat, {},
                { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
            );
            swapchainImageViews[i] = device->createImageViewUnique(viewInfo).value;
        }

        // Recreate framebuffers using the existing renderPass and depthImageView
        swapchainFramebuffers.resize(swapchainImageViews.size());
        for (size_t i = 0; i < swapchainImageViews.size(); i++) {
            std::array<vk::ImageView, 2> fbAttachments = {
                *swapchainImageViews[i],
                *depthImageView
            };
            vk::FramebufferCreateInfo framebufferInfo(
                {}, *renderPass, static_cast<uint32_t>(fbAttachments.size()),
                fbAttachments.data(), swapchainExtent.width, swapchainExtent.height, 1
            );
            swapchainFramebuffers[i] = device->createFramebufferUnique(framebufferInfo).value;
        }

        createSyncObjects();
    }

    vk::Format findDepthFormat(vk::PhysicalDevice physicalDevice) {
        std::vector<vk::Format> candidates = {
            vk::Format::eD32Sfloat,
            vk::Format::eD32SfloatS8Uint,
            vk::Format::eD24UnormS8Uint
        };

        for (auto format : candidates) {
            vk::FormatProperties props = physicalDevice.getFormatProperties(format);
            if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
                return format;
            }
        }
        throw std::runtime_error("Failed to find supported depth format!");
    }

    uint32_t findMemoryType(vk::PhysicalDevice physDevice, uint32_t typeFilter,
        vk::MemoryPropertyFlags properties) {
        vk::PhysicalDeviceMemoryProperties memProps = physDevice.getMemoryProperties();
        for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) &&
                (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        throw std::runtime_error("Failed to find suitable memory type!");
    }
}
