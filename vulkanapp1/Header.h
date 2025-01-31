// vulkan_cube.hpp
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Windows.h"

#define VULKAN_HPP_NO_EXCEPTIONS
#define ENABLE_VALIDATION_LAYERS
#include <vulkan/vulkan.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>  // For matrices and vectors
#include <glm/gtc/matrix_transform.hpp> // For transformations like rotate, lookAt, perspective

#include <array>
#include <chrono>
#include <cstring>      // For memcpy
#include <fstream>  // For std::ifstream
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>         // For exceptions (if necessary)
#include <string>   // For std::string
#include <vector>   // For std::vector

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

//#define VK_CHECK(result) if(result != vk::Result::eSuccess) { \
    throw std::runtime_error("Vulkan error at line " + std::to_string(__LINE__)); }

#define VK_CHECK(x)                                                                    \
	do                                                                                 \
	{                                                                                  \
		VkResult err = x;                                                              \
		if (err)                                                                       \
		{                                                                              \
			throw std::runtime_error("Detected Vulkan error: " + std::to_string(err)); \
		}                                                                              \
	} while (0)


namespace VulkanCube {

    struct Vertex {
        glm::vec3 pos;
        glm::vec2 texCoord;

        static vk::VertexInputBindingDescription getBindingDescription() {
            return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex };
        }

        static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions() {
            return { {
                {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)},
                {1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord)}
            } };
        }
    };

    const std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}}
    };

    const std::vector<uint16_t> indices = {
        0,1,2, 2,3,0, 4,5,6, 6,7,4,
        0,4,7, 7,3,0, 1,5,6, 6,2,1,
        0,1,5, 5,4,0, 3,2,6, 6,7,3
    };

    struct UniformBufferObject {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    // Application Class
    class Application {
    public:
        void run() {
            initWindow();
            initVulkan();
            mainLoop();
            cleanup();
        }

    private:
        // Window management
        GLFWwindow* window;

        // Vulkan instance and surface
        vk::UniqueInstance instance;
        vk::UniqueSurfaceKHR surface;

        // Vulkan debugging tools
        vk::UniqueDebugUtilsMessengerEXT debugMessenger;

        // Vulkan device and queues
        vk::PhysicalDevice physicalDevice;
        vk::UniqueDevice device;
        vk::Queue graphicsQueue;
        vk::Queue presentQueue;
        QueueFamilyIndices queueFamilyIndices;  // Store queue family indices

        // Swapchain and image views
        vk::UniqueSwapchainKHR swapChain;
        std::vector<vk::UniqueImage> swapChainImages;
        vk::Format swapChainImageFormat;
        vk::Extent2D swapChainExtent;
        std::vector<vk::UniqueImageView> swapChainImageViews;

        // Render pass and pipelines
        vk::UniqueRenderPass renderPass;
        vk::UniqueDescriptorSetLayout descriptorSetLayout;
        vk::UniquePipelineLayout pipelineLayout;
        vk::UniquePipeline graphicsPipeline;

        // Buffers (vertex, index, uniform, texture)
        vk::UniqueBuffer vertexBuffer;
        vk::UniqueDeviceMemory vertexBufferMemory;
        vk::UniqueBuffer indexBuffer;
        vk::UniqueDeviceMemory indexBufferMemory;
        vk::UniqueBuffer uniformBuffer;
        vk::UniqueDeviceMemory uniformBufferMemory;
        void* uniformBufferMapped;  // Pointer to mapped memory of uniform buffer

        // In Application class declaration
        std::vector<vk::UniqueDeviceMemory> uniformBuffersMemory; // 🔥 Add this line
        std::vector<void*> uniformBuffersMapped; // Add this to the Application class

        // Textures (image, sampler)
        vk::UniqueImage textureImage;
        vk::UniqueDeviceMemory textureImageMemory;
        vk::UniqueImageView textureImageView;
        vk::UniqueSampler textureSampler;

        // Command pool and buffers
        vk::UniqueCommandPool commandPool;
        std::vector<vk::UniqueCommandBuffer> commandBuffers;

        // Descriptor sets and pool
        vk::UniqueDescriptorPool descriptorPool;
        std::vector<vk::UniqueDescriptorSet> descriptorSets;

        // Framebuffer and depth buffer
        vk::UniqueImage depthImage;
        vk::UniqueDeviceMemory depthImageMemory;
        vk::UniqueImageView depthImageView;
        std::vector<vk::UniqueFramebuffer> framebuffers;
        vk::ClearValue clearValue;  // Clear value for rendering

        std::vector<vk::UniqueSampler> textureSamplers; // Texture samplers for each image
        std::vector<vk::UniqueBuffer> uniformBuffers;   // Uniform buffers for each frame
        std::vector<vk::UniqueImageView> textureImageViews;

        // Semaphores and fences for synchronization
        std::vector<vk::UniqueSemaphore> imageAvailableSemaphores;
        std::vector<vk::UniqueSemaphore> renderFinishedSemaphores;
        std::vector<vk::UniqueFence> inFlightFences;


        std::vector<vk::UniqueImage> textures;                   // This will hold the vk::Image objects for the textures
        vk::Format textureFormat = vk::Format::eR8G8B8A8Srgb;  // This is an example texture format, adjust as needed

        // Frame tracking and resizing
        size_t currentFrame = 0;
        bool framebufferResized = false;

        // Vulkan extensions required
        const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        void initWindow() {
            glfwInit();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            window = glfwCreateWindow(800, 600, "Vulkan Cube", nullptr, nullptr);
            glfwSetWindowUserPointer(window, this);
            glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        }

        static void framebufferResizeCallback(GLFWwindow* window, int, int) {
            auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
            app->framebufferResized = true;
        }

        void initVulkan() {

            createInstance();
            createSurface(); // Ensure the surface is created before using it

#ifdef _DEBUG
            setupDebugMessenger();
#endif
            physicalDevice = pickPhysicalDevice(); // Select a suitable physical device
            if (!physicalDevice) {
                std::cerr << "Failed to pick a physical device!" << std::endl;
                return;
            }

            initializeQueueFamilies(physicalDevice, *surface); // Initialize queue family indices
            createLogicalDevice(); // Create logical device and retrieve queues
            createSwapChain();
            createImageViews();
            createRenderPass();
            createDescriptorSetLayout();
            createGraphicsPipeline();
            createCommandPool(); // Ensure queue family indices are set up
            createDepthResources();
            createFramebuffers();
            createTextureImage();
            createTextureImageView();
            createTextureSampler();
            createVertexBuffer();
            createIndexBuffer();
            createUniformBuffers();
            createDescriptorPool();
            createDescriptorSets();
            createCommandBuffers();
            createSyncObjects();
        }

        void createInstance() {
            if (!glfwInit()) {
                throw std::runtime_error("Failed to initialize GLFW");
            }

            // Get GLFW extensions
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

            // Conditional validation layers (enable only in debug builds)
            std::vector<const char*> validationLayers;

            // Create Application Info structure
            vk::ApplicationInfo appInfo{};
            appInfo.pApplicationName = "Vulkan App";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "No Engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_0;

            // Instance Create Info
            vk::InstanceCreateInfo createInfo{};
            createInfo.pApplicationInfo = &appInfo;
            createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.data();

#ifdef _DEBUG
            // Check if validation layer is supported only in debug mode
            std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties().value;
            bool validationLayerFound = false;

            for (const auto& layer : availableLayers) {
                if (strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0) {
                    validationLayerFound = true;
                    break;
                }
            }

            if (validationLayerFound) {
                validationLayers.push_back("VK_LAYER_KHRONOS_validation");
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // For debug messaging
            }
            else {
                std::cerr << "Validation layers requested but not available!" << std::endl;
            }
#endif

            // Set enabled layers if any
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.empty() ? nullptr : validationLayers.data();

            // Create Vulkan Instance
            try {
                vk::ResultValue<vk::UniqueInstance> result = vk::createInstanceUnique(createInfo);
                if (result.result != vk::Result::eSuccess) {
                    throw std::runtime_error("Failed to create Vulkan instance: " + vk::to_string(result.result));
                }
                instance = std::move(result.value);
            }
            catch (const std::exception& e) {
                throw std::runtime_error("Vulkan instance creation failed: " + std::string(e.what()));
            }
             
     



           // // 1. Initialize GLFW
           // if (!glfwInit()) {
           //     throw std::runtime_error("Failed to initialize GLFW");
           // }

           // // 2. Get GLFW extensions
           // uint32_t glfwExtensionCount = 0;
           // const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
           // std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

           // // 4. Application Info
           // VkApplicationInfo appInfo{};
           // appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
           // appInfo.pApplicationName = "Vulkan App";
           // appInfo.apiVersion = VK_API_VERSION_1_0;

           // // 6. Instance Create Info
           // VkInstanceCreateInfo createInfo{};
           // createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
           // createInfo.pApplicationInfo = &appInfo;
           // createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
           // createInfo.ppEnabledExtensionNames = extensions.data();

           // // Create the Vulkan instance
           //// VK_CHECK(
           // VkInstance instan = VK_NULL_HANDLE;
           // VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instan));
        }

        std::vector<const char*> getRequiredExtensions() {
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            if (!glfwExtensions) {
                throw std::runtime_error("Failed to get required GLFW extensions!");
            }

            std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            return extensions;
        }

        void setupDebugMessenger() {
            vk::DebugUtilsMessengerCreateInfoEXT createInfo;
            createInfo.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
            createInfo.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance);
            createInfo.setPfnUserCallback(debugCallback);

            // No need to check vk::Result, an exception will be thrown if this fails
            auto result = instance->createDebugUtilsMessengerEXTUnique(createInfo);
            if (result.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create debug messenger!");
            }
            debugMessenger = std::move(result.value);

        }

        vk::PhysicalDevice pickPhysicalDevice() {
            // Enumerate physical devices
            auto result = instance->enumeratePhysicalDevices();

            // Check if the enumeration was successful
            if (result.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to enumerate physical devices!");
            }

            auto devices = result.value;
            if (devices.empty()) {
                throw std::runtime_error("Failed to find GPUs with Vulkan support!");
            }

            // Loop through available devices and find a suitable one
            for (const auto& device : devices) {
                if (isDeviceSuitable(device)) {
                    return device;
                }
            }

            throw std::runtime_error("Failed to find a suitable GPU!");
        }

        bool isDeviceSuitable(vk::PhysicalDevice device) {
            auto indices = findQueueFamilies(device, *surface);

            bool extensionsSupported = checkDeviceExtensionSupport(device);
            bool swapChainAdequate = false;

            if (extensionsSupported) {
                auto swapChainSupport = querySwapChainSupport(device);
                swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
            }

            return indices.isComplete() && extensionsSupported && swapChainAdequate;
        }

        QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
            QueueFamilyIndices indices;

            auto queueFamilies = device.getQueueFamilyProperties();
            for (uint32_t i = 0; i < queueFamilies.size(); i++) {
                // Check if the queue supports graphics
                if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) {
                    indices.graphicsFamily = i;
                }

                // Check if the queue supports presenting to the surface
                vk::ResultValue<uint32_t> support = device.getSurfaceSupportKHR(i, surface);
                if (support.result == vk::Result::eSuccess) {  // Compare the result field
                    indices.presentFamily = i;
                }

                // If both queues are found, no need to continue looping
                if (indices.isComplete()) {
                    break;
                }
            }

            return indices;
        }

        bool checkDeviceExtensionSupport(vk::PhysicalDevice device) {
            try {
                // Enumerate the available extensions
                auto availableExtensions = device.enumerateDeviceExtensionProperties(); // Returns vk::ResultValue<std::vector<vk::ExtensionProperties>>

                // Check if there was an error during enumeration
                if (availableExtensions.result != vk::Result::eSuccess) {
                    std::cerr << "Failed to enumerate device extensions!" << std::endl;
                    return false;
                }

                // Convert the ResultValue to the vector of extensions
                auto extensions = availableExtensions.value;

                // Ensure the list of required extensions is correctly initialized
                std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

                // Check which required extensions are supported by the device
                for (const auto& extension : extensions) {
                    requiredExtensions.erase(extension.extensionName);
                }

                // If required extensions are still missing, report them
                if (!requiredExtensions.empty()) {
                    std::cerr << "Missing required extensions: " << std::endl;
                    for (const auto& ext : requiredExtensions) {
                        std::cerr << "- " << ext << std::endl;
                    }
                    return false;
                }

                // All required extensions are available
                return true;
            }
            catch (const std::exception& e) {
                std::cerr << "Error checking device extension support: " << e.what() << std::endl;
                return false;
            }
        }

        void createLogicalDevice() {
            QueueFamilyIndices indices = findQueueFamilies(physicalDevice, *surface);

            // Ensure both queue families are valid
            if (!indices.isComplete()) {
                throw std::runtime_error("Queue families are not complete!");
            }

            std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
            std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

            float queuePriority = 1.0f;
            for (uint32_t queueFamily : uniqueQueueFamilies) {
                queueCreateInfos.push_back({
                    {}, // flags
                    queueFamily, // queueFamilyIndex
                    1, // queueCount
                    &queuePriority // pQueuePriorities
                    });
            }

            vk::PhysicalDeviceFeatures deviceFeatures;
            deviceFeatures.samplerAnisotropy = VK_TRUE; // Enable anisotropic filtering

            // Set up the logical device create info
            vk::DeviceCreateInfo createInfo(
                {}, // flags
                static_cast<uint32_t>(queueCreateInfos.size()), queueCreateInfos.data(),
                0, nullptr, // No enabled layers
                static_cast<uint32_t>(deviceExtensions.size()), deviceExtensions.data(),
                &deviceFeatures
            );

            // Attempt to create the logical device
            try {
                auto result = physicalDevice.createDeviceUnique(createInfo);
                device = std::move(result.value);
            }
            catch (const std::exception& e) {
                throw std::runtime_error("Failed to create logical device: " + std::string(e.what()));
            }

            // Retrieve the queues from the device
            graphicsQueue = device->getQueue(indices.graphicsFamily.value(), 0);
            presentQueue = device->getQueue(indices.presentFamily.value(), 0);
        }

        // --- Swap Chain Creation ---
        void createSwapChain() {
            // Query support details
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

            // Choose the surface format, present mode, and swap chain extent
            vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
            vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
            swapChainExtent = chooseSwapExtent(swapChainSupport.capabilities);

            // Set the number of images in the swap chain
            uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
            if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
                imageCount = swapChainSupport.capabilities.maxImageCount;
            }

            // Create swapchain info struct
            vk::SwapchainCreateInfoKHR createInfo(
                {}, surface.get(), imageCount, // surface, image count
                surfaceFormat.format, surfaceFormat.colorSpace, // format and color space
                swapChainExtent, 1, // extent, layers
                vk::ImageUsageFlagBits::eColorAttachment, // image usage
                vk::SharingMode::eExclusive, 0, nullptr, // queue sharing
                swapChainSupport.capabilities.currentTransform, // transform
                vk::CompositeAlphaFlagBitsKHR::eOpaque, // composite alpha
                presentMode, VK_TRUE // present mode and clipped
            );

            // Get the queue families for graphics and present
            QueueFamilyIndices indices = findQueueFamilies(physicalDevice, *surface);
            uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

            // Handle concurrent sharing mode if necessary
            if (indices.graphicsFamily != indices.presentFamily) {
                createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices = queueFamilyIndices;
            }

            // Create the swap chain
            auto result = device->createSwapchainKHRUnique(createInfo);
            if (result.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create swap chain!");
            }

            // Move the swapchain handle into the unique pointer
            swapChain = std::move(result.value);

            // Get swap chain images and store them in a vector
            auto swapChainImagesResult = device->getSwapchainImagesKHR(*swapChain);
            if (swapChainImagesResult.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to get swap chain images!");
            }

            // Convert vk::Image handles to vk::UniqueImage smart pointers
            swapChainImages.resize(swapChainImagesResult.value.size());
            for (size_t i = 0; i < swapChainImagesResult.value.size(); ++i) {
                swapChainImages[i] = vk::UniqueImage(swapChainImagesResult.value[i]);
            }

            swapChainImageFormat = surfaceFormat.format;

            // Optionally, you can store the images in vk::UniqueImageViews as well if needed.
        }

        struct SwapChainSupportDetails {
            vk::SurfaceCapabilitiesKHR capabilities;
            std::vector<vk::SurfaceFormatKHR> formats;
            std::vector<vk::PresentModeKHR> presentModes;
        };

        SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device) {
            // Query surface capabilities
            auto capabilitiesResult = device.getSurfaceCapabilitiesKHR(*surface);
            if (capabilitiesResult.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to get surface capabilities!");
            }
            vk::SurfaceCapabilitiesKHR capabilities = capabilitiesResult.value;

            // Query surface formats
            auto formatsResult = device.getSurfaceFormatsKHR(*surface);
            if (formatsResult.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to get surface formats!");
            }
            std::vector<vk::SurfaceFormatKHR> formats = formatsResult.value;

            // Query surface present modes
            auto presentModesResult = device.getSurfacePresentModesKHR(*surface);
            if (presentModesResult.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to get surface present modes!");
            }
            std::vector<vk::PresentModeKHR> presentModes = presentModesResult.value;

            // Return the query results wrapped in SwapChainSupportDetails
            return { capabilities, formats, presentModes };
        }

        vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats) {
            for (const auto& format : formats) {
                if (format.format == vk::Format::eB8G8R8A8Srgb &&
                    format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                    return format;
                }
            }
            return formats[0];
        }

        vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& modes) {
            for (const auto& mode : modes) {
                if (mode == vk::PresentModeKHR::eMailbox) return mode;
            }
            return vk::PresentModeKHR::eFifo;
        }

        vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
            if (capabilities.currentExtent.width != UINT32_MAX) {
                return capabilities.currentExtent;
            }
            else {
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
        }

        // --- Image Views ---
        void createImageViews() {
            swapChainImageViews.resize(swapChainImages.size());
            for (size_t i = 0; i < swapChainImages.size(); i++) {
                swapChainImageViews[i] = createImageViewUnique(swapChainImages[i], swapChainImageFormat);
            }
        }

        vk::UniqueImageView createImageViewUnique(const vk::UniqueImage& image, vk::Format format,
            vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor) {

            // Set up the image view create info
            vk::ImageViewCreateInfo viewInfo({}, *image, vk::ImageViewType::e2D, format,
                vk::ComponentMapping{}, // Default component mapping
                vk::ImageSubresourceRange(aspectFlags, 0, 1, 0, 1)); // Image range

            // Check if device is valid and create the image view
            if (!device) {
                throw std::runtime_error("Device is not initialized!");
            }

            // Create the image view and return it
            auto result = device->createImageViewUnique(viewInfo);

            // Return the image view extracted from the result
            return std::move(result.value);
        }

        // --- Render Pass ---
        void createRenderPass() {
            vk::AttachmentDescription colorAttachment(
                {}, swapChainImageFormat, vk::SampleCountFlagBits::e1,
                vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
                vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR
            );

            vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);

            vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics,
                0, nullptr, 1, &colorAttachmentRef);

            vk::SubpassDependency dependency(
                VK_SUBPASS_EXTERNAL, 0,
                vk::PipelineStageFlagBits::eColorAttachmentOutput,
                vk::PipelineStageFlagBits::eColorAttachmentOutput,
                {}, vk::AccessFlagBits::eColorAttachmentWrite);

            vk::RenderPassCreateInfo renderPassInfo({}, 1, &colorAttachment, 1, &subpass, 1, &dependency);

            // Create the render pass and check for success
            auto result = device->createRenderPassUnique(renderPassInfo);
            if (result.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create render pass");
            }

            // Move the created render pass into the member variable
            renderPass = std::move(result.value);
        }

        // --- Graphics Pipeline ---
        void createGraphicsPipeline() {
            auto vertShaderCode = readFile("vert.spv");
            auto fragShaderCode = readFile("frag.spv");

            // Pass the device when creating shader modules
            vk::UniqueShaderModule vertShaderModule = createShaderModule(device, vertShaderCode);
            vk::UniqueShaderModule fragShaderModule = createShaderModule(device, fragShaderCode);

            vk::PipelineShaderStageCreateInfo vertShaderStageInfo({}, vk::ShaderStageFlagBits::eVertex, *vertShaderModule, "main");
            vk::PipelineShaderStageCreateInfo fragShaderStageInfo({}, vk::ShaderStageFlagBits::eFragment, *fragShaderModule, "main");
            vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

            auto bindingDescription = Vertex::getBindingDescription();
            auto attributeDescriptions = Vertex::getAttributeDescriptions();

            vk::PipelineVertexInputStateCreateInfo vertexInputInfo(
                {}, 1, &bindingDescription,
                static_cast<uint32_t>(attributeDescriptions.size()), attributeDescriptions.data());

            vk::PipelineInputAssemblyStateCreateInfo inputAssembly({}, vk::PrimitiveTopology::eTriangleList);

            vk::Viewport viewport(0.0f, 0.0f,
                static_cast<float>(swapChainExtent.width),
                static_cast<float>(swapChainExtent.height),
                0.0f, 1.0f);
            vk::Rect2D scissor({ 0, 0 }, swapChainExtent);
            vk::PipelineViewportStateCreateInfo viewportState({}, 1, &viewport, 1, &scissor);

            vk::PipelineRasterizationStateCreateInfo rasterizer({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill,
                vk::CullModeFlagBits::eBack, vk::FrontFace::eClockwise);

            vk::PipelineMultisampleStateCreateInfo multisampling;

            vk::PipelineColorBlendAttachmentState colorBlendAttachment;
            colorBlendAttachment.colorWriteMask =
                vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

            vk::PipelineColorBlendStateCreateInfo colorBlending(
                {}, VK_FALSE, vk::LogicOp::eCopy, 1, &colorBlendAttachment);

            // Creating pipeline layout with the unique device
            vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, 1, &*descriptorSetLayout);
            auto pipelineLayoutResult = device->createPipelineLayoutUnique(pipelineLayoutInfo);
            if (pipelineLayoutResult.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create pipeline layout!");
            }

            // Move the value from the ResultValue to the pipelineLayout
            pipelineLayout = std::move(pipelineLayoutResult.value);

            // Create the graphics pipeline info structure
            vk::GraphicsPipelineCreateInfo pipelineInfo(
                {}, 2, shaderStages, &vertexInputInfo, &inputAssembly,
                nullptr, &viewportState, &rasterizer, &multisampling,
                nullptr, &colorBlending, nullptr,
                *pipelineLayout, *renderPass);

            // Creating graphics pipeline with the unique device
            auto pipelineResult = device->createGraphicsPipelineUnique(nullptr, pipelineInfo);
            if (pipelineResult.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create graphics pipeline!");
            }

            // Move the value from the ResultValue to the graphicsPipeline
            graphicsPipeline = std::move(pipelineResult.value);

        }

        vk::UniqueShaderModule createShaderModule(vk::UniqueDevice& device, const std::vector<char>& code) {
            // Ensure SPIR-V bytecode is properly aligned
            assert(code.size() % 4 == 0 && "SPIR-V bytecode size must be a multiple of 4");

            // Create shader module create info
            vk::ShaderModuleCreateInfo createInfo(
                {}, // Flags (reserved, must be 0)
                code.size(),
                reinterpret_cast<const uint32_t*>(code.data()) // SPIR-V code pointer
            );

            // Create and return the shader module using the UniqueDevice
            vk::ShaderModule shaderModule;
            if (device->createShaderModule(&createInfo, nullptr, &shaderModule) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create shader module!");
            }

            return vk::UniqueShaderModule(shaderModule, *device);
        }

        void initializeQueueFamilies(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface) {
            queueFamilyIndices = findQueueFamilies(physicalDevice, surface);
            if (!queueFamilyIndices.isComplete()) {
                throw std::runtime_error("Failed to find required queue families!");
            }
        }

        void createCommandPool() {
            // Ensure that the graphics queue family index is valid
            if (!queueFamilyIndices.graphicsFamily.has_value()) {
                throw std::runtime_error("Graphics family index is not set!");
            }

            vk::CommandPoolCreateInfo poolInfo(
                vk::CommandPoolCreateFlagBits::eResetCommandBuffer,  // Allow resetting command buffers
                queueFamilyIndices.graphicsFamily.value()           // Use the graphics queue family
            );

            // Create the command pool and access the unique pool with .value
            commandPool = device->createCommandPoolUnique(poolInfo).value;
        }

        // --- Command Buffers ---
        void createCommandBuffers() {
            // Resize commandBuffers to match the number of swapChainImageViews
            commandBuffers.resize(swapChainImageViews.size());

            // Allocate command buffers
            vk::CommandBufferAllocateInfo allocInfo(*commandPool, vk::CommandBufferLevel::ePrimary,
                static_cast<uint32_t>(commandBuffers.size()));

            // Allocate command buffers
            auto result = device->allocateCommandBuffersUnique(allocInfo);
            if (result.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to allocate command buffers!");
            }

            // Move the allocated command buffers into the commandBuffers vector
            for (size_t i = 0; i < result.value.size(); ++i) {
                commandBuffers[i] = std::move(result.value[i]);
            }

            // Define the clear values
            std::vector<vk::ClearValue> clearValues;
            clearValues.push_back(vk::ClearValue().setColor(vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f})));
            clearValues.push_back(vk::ClearValue().setDepthStencil({ 1.0f, 0 }));

            // Loop through and record command buffers
            for (size_t i = 0; i < commandBuffers.size(); i++) {
                vk::CommandBufferBeginInfo beginInfo;
                commandBuffers[i]->begin(beginInfo);

                vk::RenderPassBeginInfo renderPassInfo(
                    *renderPass,
                    *framebuffers[i],
                    vk::Rect2D({ 0, 0 }, swapChainExtent),
                    static_cast<uint32_t>(clearValues.size()),
                    clearValues.data()
                );

                commandBuffers[i]->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
                commandBuffers[i]->bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);
                commandBuffers[i]->bindVertexBuffers(0, { *vertexBuffer }, { 0 });
                commandBuffers[i]->bindIndexBuffer(*indexBuffer, 0, vk::IndexType::eUint16);
                commandBuffers[i]->bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics,
                    *pipelineLayout,
                    0,
                    { *descriptorSets[i] }, // Ensure descriptorSets[i] is valid
                    {}
                );
                commandBuffers[i]->drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
                commandBuffers[i]->endRenderPass();
                commandBuffers[i]->end();
            }
        }

        // --- Texture Creation ---
        void createTextureImage() {
            int texWidth, texHeight, texChannels;
            stbi_uc* pixels = stbi_load("texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            if (!pixels) throw std::runtime_error("Failed to load texture image!");

            vk::DeviceSize imageSize = texWidth * texHeight * 4;
            vk::UniqueBuffer stagingBuffer;
            vk::UniqueDeviceMemory stagingBufferMemory;
            createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                stagingBuffer, stagingBufferMemory);

            // Map the staging buffer memory
            vk::ResultValue<void*> result = device->mapMemory(*stagingBufferMemory, 0, imageSize);
            if (result.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to map memory!");
            }
            void* data = result.value;

            memcpy(data, pixels, static_cast<size_t>(imageSize));
            device->unmapMemory(*stagingBufferMemory);
            stbi_image_free(pixels);

            createImage(texWidth, texHeight, vk::Format::eR8G8B8A8Srgb,
                vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                vk::MemoryPropertyFlagBits::eDeviceLocal, textureImage, textureImageMemory);

            transitionImageLayout(*textureImage, vk::Format::eR8G8B8A8Srgb,
                vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
            copyBufferToImage(stagingBuffer, *textureImage, texWidth, texHeight);
            transitionImageLayout(*textureImage, vk::Format::eR8G8B8A8Srgb,
                vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
        }

        // --- Texture Image View ---
        void createTextureImageView() {
            vk::ImageViewCreateInfo viewInfo = {};
            viewInfo.image = *textureImage;  // Correctly dereference the unique_ptr for vk::Image
            viewInfo.viewType = vk::ImageViewType::e2D;
            viewInfo.format = vk::Format::eR8G8B8A8Srgb;
            viewInfo.components = vk::ComponentMapping();
            viewInfo.subresourceRange = vk::ImageSubresourceRange(
                vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

            // Create ImageView
            auto result = device->createImageViewUnique(viewInfo);
            if (result.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create image view!");
            }

            textureImageView = std::move(result.value);  // Move the created image view into textureImageView
        }

        void createImage(uint32_t width, uint32_t height, vk::Format format,
            vk::ImageTiling tiling, vk::ImageUsageFlags usage,
            vk::MemoryPropertyFlags properties,
            vk::UniqueImage& image, vk::UniqueDeviceMemory& imageMemory) {

            vk::ImageCreateInfo imageInfo(
                {},
                vk::ImageType::e2D,
                format,
                vk::Extent3D(width, height, 1),
                1,
                1,
                vk::SampleCountFlagBits::e1,
                tiling,
                usage,
                vk::SharingMode::eExclusive
            );

            // Create the image
            auto result = device->createImageUnique(imageInfo);
            if (result.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create image!");
            }
            image = std::move(result.value);  // Move the created image into image

            // Get memory requirements for the image
            vk::MemoryRequirements memRequirements = device->getImageMemoryRequirements(*image);

            // Allocate memory for the image
            vk::MemoryAllocateInfo allocInfo(memRequirements.size,
                findMemoryType(memRequirements.memoryTypeBits, properties));

            auto allocResult = device->allocateMemoryUnique(allocInfo);
            if (allocResult.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to allocate image memory!");
            }
            imageMemory = std::move(allocResult.value);  // Move the allocated memory into imageMemory

            // Bind the image memory
            device->bindImageMemory(*image, *imageMemory, 0);
        }

        void createVertexBuffer() {
            vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

            vk::BufferCreateInfo bufferInfo(
                {},
                bufferSize,
                vk::BufferUsageFlagBits::eVertexBuffer,
                vk::SharingMode::eExclusive
            );

            // Create the vertex buffer
            auto result = device->createBufferUnique(bufferInfo);
            if (result.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create vertex buffer!");
            }
            vertexBuffer = std::move(result.value);  // Move the created buffer into vertexBuffer

            // Allocate memory for the buffer
            vk::MemoryRequirements memRequirements = device->getBufferMemoryRequirements(*vertexBuffer);
            vk::MemoryAllocateInfo allocInfo(
                memRequirements.size,
                findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
            );

            auto allocResult = device->allocateMemoryUnique(allocInfo);
            if (allocResult.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to allocate vertex buffer memory!");
            }
            vertexBufferMemory = std::move(allocResult.value);  // Move the allocated memory into vertexBufferMemory

            // Bind the memory to the buffer
            device->bindBufferMemory(*vertexBuffer, *vertexBufferMemory, 0);

            // Map the memory and copy vertex data
            auto mapResult = device->mapMemory(*vertexBufferMemory, 0, bufferSize);
            if (mapResult.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to map vertex buffer memory!");
            }
            void* data = mapResult.value;  // Extract the void* pointer

            memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
            device->unmapMemory(*vertexBufferMemory);
        }

        void createIndexBuffer() {
            vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

            vk::BufferCreateInfo bufferInfo(
                {},
                bufferSize,
                vk::BufferUsageFlagBits::eIndexBuffer,
                vk::SharingMode::eExclusive
            );

            // Create the index buffer
            auto result = device->createBufferUnique(bufferInfo);
            if (result.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create index buffer!");
            }
            indexBuffer = std::move(result.value);  // Move the created buffer into indexBuffer

            // Allocate memory for the buffer
            vk::MemoryRequirements memRequirements = device->getBufferMemoryRequirements(*indexBuffer);
            vk::MemoryAllocateInfo allocInfo(
                memRequirements.size,
                findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
            );

            auto allocResult = device->allocateMemoryUnique(allocInfo);
            if (allocResult.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to allocate index buffer memory!");
            }
            indexBufferMemory = std::move(allocResult.value);  // Move the allocated memory into indexBufferMemory

            // Bind the memory to the buffer
            device->bindBufferMemory(*indexBuffer, *indexBufferMemory, 0);

            // Map the memory and copy index data
            auto mapResult = device->mapMemory(*indexBufferMemory, 0, bufferSize);
            if (mapResult.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to map index buffer memory!");
            }
            void* data = mapResult.value;  // Extract the void* pointer

            memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
            device->unmapMemory(*indexBufferMemory);
        }


        // --- Utility Functions ---
        uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
            vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();
            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                if ((typeFilter & (1 << i)) &&
                    (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }
            throw std::runtime_error("Failed to find suitable memory type!");
        }

        void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
            vk::MemoryPropertyFlags properties, vk::UniqueBuffer& buffer,
            vk::UniqueDeviceMemory& bufferMemory) {
            vk::BufferCreateInfo bufferInfo({}, size, usage);

            // Create the buffer
            auto result = device->createBufferUnique(bufferInfo);
            if (result.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create buffer!");
            }
            buffer = std::move(result.value);  // Move the created buffer into buffer

            // Allocate memory for the buffer
            vk::MemoryRequirements memRequirements = device->getBufferMemoryRequirements(*buffer);
            vk::MemoryAllocateInfo allocInfo(memRequirements.size,
                findMemoryType(memRequirements.memoryTypeBits, properties));

            auto allocResult = device->allocateMemoryUnique(allocInfo);
            if (allocResult.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to allocate buffer memory!");
            }
            bufferMemory = std::move(allocResult.value);  // Move the allocated memory into bufferMemory

            // Bind the memory to the buffer
            device->bindBufferMemory(*buffer, *bufferMemory, 0);
        }

        void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) {
            // Allocate command buffer
            vk::CommandBufferAllocateInfo allocInfo(*commandPool, vk::CommandBufferLevel::ePrimary, 1);

            // Allocate command buffers and check result
            auto result = device->allocateCommandBuffersUnique(allocInfo);

            if (result.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to allocate command buffer!");
            }

            vk::UniqueCommandBuffer commandBuffer = std::move(result.value[0]);

            // Begin command buffer recording
            vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
            commandBuffer->begin(beginInfo);

            // Perform the buffer copy
            vk::BufferCopy copyRegion(0, 0, size);  // From offset 0 to offset 0, with the specified size
            commandBuffer->copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

            // End command buffer recording
            commandBuffer->end();

            // Submit command buffer to the graphics queue
            vk::SubmitInfo submitInfo(0, nullptr, nullptr, 1, &*commandBuffer);

            // Submit with error checking
            try {
                graphicsQueue.submit(1, &submitInfo, nullptr);
            }
            catch (const std::exception& e) {
                std::cerr << "Error during command buffer submission: " << e.what() << std::endl;
            }

            // Wait for the queue to finish processing the commands
            graphicsQueue.waitIdle();
        }

        // Surface creation
        void createSurface() {
            if (!glfwVulkanSupported()) {
                throw std::runtime_error("Vulkan is not supported by GLFW!");
            }

            if (!window) {
                throw std::runtime_error("GLFW window is not created!");
            }

            VkSurfaceKHR rawSurface;
            VkResult result = glfwCreateWindowSurface(*instance, window, nullptr, &rawSurface);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("Failed to create window surface! Vulkan error: " + std::to_string(result));
            }

            // Wrap the raw Vulkan handle in vk::UniqueSurfaceKHR
            surface = vk::UniqueSurfaceKHR(rawSurface, *instance);
        }

        // Depth resources
        void createDepthResources() {
            vk::Format depthFormat = findDepthFormat();
            createImage(swapChainExtent.width, swapChainExtent.height, depthFormat,
                vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment,
                vk::MemoryPropertyFlagBits::eDeviceLocal, depthImage, depthImageMemory);
            depthImageView = createImageViewUnique(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);
        }

        vk::Format findDepthFormat() {
            return findSupportedFormat(
                { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
                vk::ImageTiling::eOptimal,
                vk::FormatFeatureFlagBits::eDepthStencilAttachment
            );
        }

        vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling,
            vk::FormatFeatureFlags features) {
            for (vk::Format format : candidates) {
                vk::FormatProperties props = physicalDevice.getFormatProperties(format);
                if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
                    return format;
                }
                else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
                    return format;
                }
            }
            throw std::runtime_error("Failed to find supported format!");
        }

        // Framebuffers
        void createFramebuffers() {
            framebuffers.resize(swapChainImageViews.size());
            for (size_t i = 0; i < swapChainImageViews.size(); i++) {
                std::array<vk::ImageView, 2> attachments = {
                    *swapChainImageViews[i],
                    *depthImageView
                };

                vk::FramebufferCreateInfo framebufferInfo(
                    {}, *renderPass,
                    static_cast<uint32_t>(attachments.size()), attachments.data(),
                    swapChainExtent.width, swapChainExtent.height, 1);

                // Create framebuffer and check result
                auto result = device->createFramebufferUnique(framebufferInfo);
                if (result.result != vk::Result::eSuccess) {
                    throw std::runtime_error("Failed to create framebuffer!");
                }
                framebuffers[i] = std::move(result.value);
            }
        }

        // Texture sampler
        void createTextureSampler() {
            vk::SamplerCreateInfo samplerInfo(
                {}, vk::Filter::eLinear, vk::Filter::eLinear,
                vk::SamplerMipmapMode::eLinear,
                vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
                vk::SamplerAddressMode::eRepeat, 0.0f, VK_TRUE, 16.0f,
                VK_FALSE, vk::CompareOp::eAlways, 0.0f, 0.0f,
                vk::BorderColor::eIntOpaqueBlack, VK_FALSE);

            // Create sampler and check result
            auto result = device->createSamplerUnique(samplerInfo);
            if (result.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create texture sampler!");
            }
            textureSampler = std::move(result.value);
        }

        // Uniform PLURAL buffers
        void createUniformBuffers() {
            uniformBuffers.resize(swapChainImages.size());
            uniformBuffersMemory.resize(swapChainImages.size()); // Store memory
            uniformBuffersMapped.resize(swapChainImages.size());

            for (size_t i = 0; i < swapChainImages.size(); i++) {
                vk::BufferCreateInfo bufferInfo({}, sizeof(UniformBufferObject), vk::BufferUsageFlagBits::eUniformBuffer);
                uniformBuffers[i] = device->createBufferUnique(bufferInfo).value;

                vk::MemoryRequirements memRequirements = device->getBufferMemoryRequirements(*uniformBuffers[i]);
                vk::MemoryAllocateInfo allocInfo(
                    memRequirements.size,
                    findMemoryType(memRequirements.memoryTypeBits,
                        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
                );

                // 🔥 Store the memory in the vector
                uniformBuffersMemory[i] = device->allocateMemoryUnique(allocInfo).value;
                device->bindBufferMemory(*uniformBuffers[i], *uniformBuffersMemory[i], 0);

                // Map the memory
                auto mapResult = device->mapMemory(*uniformBuffersMemory[i], 0, sizeof(UniformBufferObject));
                if (mapResult.result != vk::Result::eSuccess) {
                    throw std::runtime_error("Failed to map uniform buffer memory!");
                }
                uniformBuffersMapped[i] = mapResult.value;
            }
        }

        // Update uniform buffer
        void updateUniformBuffer(uint32_t currentImage) {
            static auto startTime = std::chrono::high_resolution_clock::now();
            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float>(currentTime - startTime).count();

            UniformBufferObject ubo{};
            ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
            ubo.proj[1][1] *= -1;

            memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
        }

        // Texture PLURAL samplers
        void createTextureSamplers() {
            textureSamplers.resize(swapChainImages.size());

            // Create texture samplers for each image (this is an example, adjust as needed)
            for (size_t i = 0; i < swapChainImages.size(); i++) {
                vk::SamplerCreateInfo samplerInfo({}, vk::Filter::eLinear, vk::Filter::eLinear,
                    vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eRepeat,
                    vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
                    0.0f, VK_FALSE, 16.0f, VK_FALSE, vk::CompareOp::eNever,
                    0.0f, 0.0f, vk::BorderColor::eIntOpaqueBlack, VK_FALSE);

                auto result = device->createSamplerUnique(samplerInfo);
                if (result.result != vk::Result::eSuccess) {
                    throw std::runtime_error("Failed to create texture sampler!");
                }
                textureSamplers[i] = std::move(result.value);  // Move ownership
            }
        }

        // Example function to create image views for textures
        void createTextureImageViews() {
            textureImageViews.resize(textures.size());  // Assuming textures is a vector of vk::UniqueImage objects

            for (size_t i = 0; i < textures.size(); i++) {
                // Create an image view for each texture using the previously defined method
                textureImageViews[i] = createImageViewUnique(textures[i], textureFormat);  // Assuming textureFormat is set
            }
        }

        // Allocate Descriptor Sets
        void createDescriptorSets() {
            std::vector<vk::DescriptorSetLayout> layouts(swapChainImages.size(), *descriptorSetLayout);
            vk::DescriptorSetAllocateInfo allocInfo(*descriptorPool, static_cast<uint32_t>(swapChainImages.size()), layouts.data());

            auto result = device->allocateDescriptorSetsUnique(allocInfo);
            if (result.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to allocate descriptor sets!");
            }
            descriptorSets = std::move(result.value);

            for (size_t i = 0; i < swapChainImages.size(); ++i) {
                vk::DescriptorBufferInfo bufferInfo(*uniformBuffers[i], 0, sizeof(UniformBufferObject));
                vk::DescriptorImageInfo imageInfo(*textureSampler, *textureImageView, vk::ImageLayout::eShaderReadOnlyOptimal);

                std::array<vk::WriteDescriptorSet, 2> descriptorWrites{};
                descriptorWrites[0] = { *descriptorSets[i], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &bufferInfo };
                descriptorWrites[1] = { *descriptorSets[i], 1, 0, 1, vk::DescriptorType::eCombinedImageSampler, &imageInfo };

                device->updateDescriptorSets(descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
            }
        }

        // Descriptor Set Layout
        void createDescriptorSetLayout() {
            std::array<vk::DescriptorSetLayoutBinding, 2> bindings{};

            // Binding for the uniform buffer (vertex shader)
            bindings[0] = { 0, vk::DescriptorType::eUniformBuffer, 1,
                            vk::ShaderStageFlagBits::eVertex };

            // Binding for the combined image sampler (fragment shader)
            bindings[1] = { 1, vk::DescriptorType::eCombinedImageSampler, 1,
                            vk::ShaderStageFlagBits::eFragment };

            vk::DescriptorSetLayoutCreateInfo layoutInfo({}, bindings.size(), bindings.data());

            // Create the descriptor set layout and check the result
            try {
                // Store the unique descriptor set layout here, Correct assignment with .value
                descriptorSetLayout = device->createDescriptorSetLayoutUnique(layoutInfo).value;
            }
            catch (const std::exception& e) {
                throw std::runtime_error("Failed to create descriptor set layout: " + std::string(e.what()));
            }
        }

        // Descriptor Pool and Descriptor Set Allocation
        void createDescriptorPool() {
            std::array<vk::DescriptorPoolSize, 2> poolSizes{};

            // Pool size for uniform buffer descriptors
            poolSizes[0] = { vk::DescriptorType::eUniformBuffer, static_cast<uint32_t>(swapChainImages.size()) };

            // Pool size for combined image sampler descriptors
            poolSizes[1] = { vk::DescriptorType::eCombinedImageSampler, static_cast<uint32_t>(swapChainImages.size()) };

            // Create DescriptorPoolCreateInfo with explicit casting
            vk::DescriptorPoolCreateInfo poolInfo(
                vk::DescriptorPoolCreateFlags(0),     // No special flags
                static_cast<uint32_t>(swapChainImages.size()), // Max sets
                static_cast<uint32_t>(poolSizes.size()),        // Number of pool sizes
                poolSizes.data()                       // Pointer to pool sizes
            );

            try {
                // Store the unique descriptor pool here
                descriptorPool = device->createDescriptorPoolUnique(poolInfo).value;
            }
            catch (const std::exception& e) {
                throw std::runtime_error("Failed to create descriptor pool: " + std::string(e.what()));
            }
        }

        // Sync objects
        void createSyncObjects() {
            // Resize vectors to hold two elements each (double buffering)
            imageAvailableSemaphores.resize(2);
            renderFinishedSemaphores.resize(2);
            inFlightFences.resize(2);

            // Semaphore and fence creation info
            vk::SemaphoreCreateInfo semaphoreInfo;
            vk::FenceCreateInfo fenceInfo(vk::FenceCreateFlagBits::eSignaled);

            for (size_t i = 0; i < 2; i++) {
                try {
                    // Create semaphores and fences using unique pointers
                    auto imageAvailableResult = device->createSemaphoreUnique(semaphoreInfo);
                    if (imageAvailableResult.result != vk::Result::eSuccess) {
                        throw std::runtime_error("Failed to create image available semaphore!");
                    }
                    imageAvailableSemaphores[i] = std::move(imageAvailableResult.value);

                    auto renderFinishedResult = device->createSemaphoreUnique(semaphoreInfo);
                    if (renderFinishedResult.result != vk::Result::eSuccess) {
                        throw std::runtime_error("Failed to create render finished semaphore!");
                    }
                    renderFinishedSemaphores[i] = std::move(renderFinishedResult.value);

                    auto fenceResult = device->createFenceUnique(fenceInfo);
                    if (fenceResult.result != vk::Result::eSuccess) {
                        throw std::runtime_error("Failed to create fence!");
                    }
                    inFlightFences[i] = std::move(fenceResult.value);

                }
                catch (const std::exception& e) {
                    std::cerr << "Error during synchronization object creation: " << e.what() << std::endl;
                    throw;  // Re-throw to propagate the error
                }
            }
        }

        // Helper function to start recording
        vk::UniqueCommandBuffer beginSingleTimeCommands() {
            vk::CommandBufferAllocateInfo allocInfo(
                *commandPool,  // Your command pool
                vk::CommandBufferLevel::ePrimary,
                1
            );

            // Allocate and begin recording
            vk::UniqueCommandBuffer cmdBuffer =
                std::move(device.get().allocateCommandBuffersUnique(allocInfo).value[0]);

            vk::CommandBufferBeginInfo beginInfo{
                vk::CommandBufferUsageFlagBits::eOneTimeSubmit
            };
            cmdBuffer->begin(beginInfo);

            return cmdBuffer; // Ownership transferred via move
        }

        // Helper to submit commands
        void endSingleTimeCommands(vk::UniqueCommandBuffer& cmdBuffer) {
            cmdBuffer->end();

            vk::SubmitInfo submitInfo;
            submitInfo.setCommandBuffers(*cmdBuffer);
            graphicsQueue.submit(submitInfo, nullptr);
            graphicsQueue.waitIdle(); // Or use fences for async operation
        }

        // Image transitions
        void transitionImageLayout(vk::Image image, vk::Format format,
            vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
            // Store the UniqueCommandBuffer BY VALUE (not reference)
            vk::UniqueCommandBuffer commandBuffer = beginSingleTimeCommands();

            vk::ImageMemoryBarrier barrier(
                {}, {}, oldLayout, newLayout, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, image,
                vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

            vk::PipelineStageFlags sourceStage;
            vk::PipelineStageFlags destinationStage;

            if (oldLayout == vk::ImageLayout::eUndefined &&
                newLayout == vk::ImageLayout::eTransferDstOptimal) {
                barrier.srcAccessMask = {};
                barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
                sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
                destinationStage = vk::PipelineStageFlagBits::eTransfer;
            }
            else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
                newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
                barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
                sourceStage = vk::PipelineStageFlagBits::eTransfer;
                destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
            }
            else {
                throw std::invalid_argument("Unsupported layout transition!");
            }

            // Use commandBuffer.get() to access the underlying VkCommandBuffer
            commandBuffer->pipelineBarrier(sourceStage, destinationStage, {}, 0, nullptr, 0, nullptr, 1, &barrier);
            endSingleTimeCommands(commandBuffer);
        }

        // Buffer/image copying
        void copyBufferToImage(vk::UniqueBuffer& buffer, vk::Image image, uint32_t width, uint32_t height) {
            vk::UniqueCommandBuffer commandBuffer = beginSingleTimeCommands();

            vk::BufferImageCopy region(
                0, 0, 0,
                vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
                { 0, 0, 0 }, { width, height, 1 });

            commandBuffer->copyBufferToImage(buffer.get(), image, vk::ImageLayout::eTransferDstOptimal, 1, &region);
            endSingleTimeCommands(commandBuffer);
        }

        // Draw frame implementation
        void drawFrame() {
            
            device->waitForFences(1, &*inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

            // Acquire image index
            uint32_t imageIndex;
            auto result = device->acquireNextImageKHR(*swapChain, UINT64_MAX,
                *imageAvailableSemaphores[currentFrame], nullptr, &imageIndex);

            // Validate image index
            if (imageIndex >= swapChainImages.size()) {
                throw std::runtime_error("Acquired image index out of bounds!");
            }

            // Update UBO only if index is valid
            updateUniformBuffer(imageIndex);

            if (result == vk::Result::eErrorOutOfDateKHR || framebufferResized) {
                framebufferResized = false;
                recreateSwapChain();
                return;
            }

            device->resetFences(1, &*inFlightFences[currentFrame]);

            vk::SubmitInfo submitInfo(
                1, &*imageAvailableSemaphores[currentFrame],
                nullptr,
                1, &*commandBuffers[imageIndex],
                1, &*renderFinishedSemaphores[currentFrame]);

            graphicsQueue.submit(1, &submitInfo, *inFlightFences[currentFrame]);

            vk::PresentInfoKHR presentInfo(
                1, &*renderFinishedSemaphores[currentFrame],
                1, &*swapChain, &imageIndex);

            result = presentQueue.presentKHR(&presentInfo);

            if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
                recreateSwapChain();
            }

            currentFrame = (currentFrame + 1) % 2;
        }

        // Swap chain recreation
        void recreateSwapChain() {
            int width = 0, height = 0;
            glfwGetFramebufferSize(window, &width, &height);
            while (width == 0 || height == 0) {
                glfwGetFramebufferSize(window, &width, &height);
                glfwWaitEvents();
            }

            device->waitIdle();

            cleanupSwapChain();

            createSwapChain();
            createImageViews();
            createRenderPass();      // Recreate render pass
            createGraphicsPipeline();// Recreate pipeline
            createDepthResources();
            createFramebuffers();
            createUniformBuffers();  // Recreate uniform buffers
            createDescriptorPool();  // Recreate descriptor pool
            createDescriptorSets();
            createCommandBuffers();
        }

        void cleanupSwapChain() {
            framebuffers.clear();
            swapChainImageViews.clear();
            swapChain.reset();

            // Clear depth resources
            depthImageView.reset();
            depthImageMemory.reset();
            depthImage.reset();

            // Clear uniform buffers
            uniformBuffers.clear();
            uniformBuffersMemory.clear(); // 🔥 Add this line
            uniformBuffersMapped.clear();
        }

        // File reading utility
        static std::vector<char> readFile(const std::string& filename) {
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

        void mainLoop() {

            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();
                drawFrame();
            }
            device->waitIdle();
        }

        void cleanup() {
            device->waitIdle();
            cleanupSwapChain();

            // Add these cleanup operations
            depthImageView.reset();
            depthImageMemory.reset();
            depthImage.reset();
            textureSampler.reset();
            textureImageView.reset();
            textureImageMemory.reset();
            textureImage.reset();
            uniformBuffer.reset();
            uniformBufferMemory.reset();
            descriptorPool.reset();
            descriptorSetLayout.reset();
            pipelineLayout.reset();
            graphicsPipeline.reset();
            renderPass.reset();
            commandPool.reset();

            for (size_t i = 0; i < 2; i++) {
                inFlightFences[i].reset();
                renderFinishedSemaphores[i].reset();
                imageAvailableSemaphores[i].reset();
            }

            device.reset();
            debugMessenger.reset();
            surface.reset();
            instance.reset();

            glfwDestroyWindow(window);
            glfwTerminate();
        }
    };
}

#ifdef VULKAN_CUBE_MAIN
int main() {
    VulkanCube::Application app;
    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
#endif