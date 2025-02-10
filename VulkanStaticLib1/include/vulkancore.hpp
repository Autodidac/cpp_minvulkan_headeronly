#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define VULKAN_HPP_NO_EXCEPTIONS
#define ENABLE_VALIDATION_LAYERS
#include <vulkan/vulkan.hpp>

#include <memory>
#include <optional>
#include <vector>
#include <array>

namespace VulkanCube {
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        bool isComplete() const;
    };

    struct SwapChainSupportDetails {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    struct Context {
        // Core members
        vk::UniqueInstance instance;
        vk::UniqueSurfaceKHR surface;
        vk::PhysicalDevice physicalDevice;
        vk::UniqueDevice device;
        vk::Queue graphicsQueue;
        vk::Queue presentQueue;
        QueueFamilyIndices queueIndices;

        // Swapchain
        vk::UniqueSwapchainKHR swapchain;
        vk::Extent2D swapchainExtent;
        vk::Format swapchainFormat;
        std::vector<vk::Image> swapchainImages;
        std::vector<vk::UniqueImageView> swapchainImageViews;
        vk::UniqueImageView depthImageView;

        // Depth resources
        vk::UniqueImage depthImage;
        vk::UniqueDeviceMemory depthImageMemory;

        // Sync objects
        std::vector<vk::UniqueSemaphore> imageAvailableSemaphores;
        std::vector<vk::UniqueSemaphore> renderFinishedSemaphores;
        std::vector<vk::UniqueFence> inFlightFences;
        uint32_t currentFrame = 0;

        // Pipeline
        vk::UniqueRenderPass renderPass;
        vk::UniquePipelineLayout pipelineLayout;
        vk::UniquePipeline graphicsPipeline;

        // Framebuffers
        std::vector<vk::UniqueFramebuffer> swapchainFramebuffers;

        // New members for features and properties:
        vk::PhysicalDeviceFeatures deviceFeatures;
        vk::PhysicalDeviceProperties deviceProperties;

        // Constants
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
        static const std::vector<const char*> deviceExtensions;

        static Context createWindow(GLFWwindow* window, bool enableValidation = false);
        void recreateSwapchain(GLFWwindow* window);
        void createSyncObjects();
        SwapChainSupportDetails querySwapChainSupport() const;
    };

    vk::Format findDepthFormat(vk::PhysicalDevice physicalDevice);
    uint32_t findMemoryType(vk::PhysicalDevice physDevice, uint32_t typeFilter,
        vk::MemoryPropertyFlags properties);
}