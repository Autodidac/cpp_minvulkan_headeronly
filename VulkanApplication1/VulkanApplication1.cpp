#include "..\VulkanStaticLib1\framework.h"
#include "..\VulkanStaticLib1\include\vulkanbuffers.hpp"
#include "..\VulkanStaticLib1\include\vulkancore.hpp"
#include "..\VulkanStaticLib1\include\vulkanpipeline.hpp"
#include "..\VulkanStaticLib1\include\vulkantextures.hpp"
#include "..\VulkanStaticLib1\include\vulkancommands.hpp"
#include "..\VulkanStaticLib1\include\vulkandescriptors.hpp"

#include <GLFW/glfw3.h>

#include <chrono>
#include <iostream>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>


const std::vector<VulkanCube::Vertex> vertices = {
    {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}},
    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}},
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}},
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}},
    {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4,
    3, 2, 6, 6, 7, 3, 0, 1, 5, 5, 4, 0,
    4, 0, 3, 3, 7, 4, 1, 5, 6, 6, 2, 1
};

class CubeApp {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;
    VulkanCube::Context context;
    VulkanCube::CommandPool commandPool;
    VulkanCube::Texture texture;
    VulkanCube::BufferPackage vertexBuffer;
    VulkanCube::BufferPackage indexBuffer;
    VulkanCube::BufferPackage uniformBuffer;
    VulkanCube::DescriptorSets descriptorSets;
    VulkanCube::UniformBufferObject ubo{};
    bool framebufferResized = false;

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(800, 600, "Vulkan Cube", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<CubeApp*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    void initVulkan() {
        context = VulkanCube::Context::create(window, true);
        commandPool = VulkanCube::CommandPool::create(context, 2);
        texture = VulkanCube::Texture::loadFromFile(context, commandPool, "texture.jpg");

        // Load shaders
        auto vertShaderCode = VulkanCube::readFile("shader.vert.spv");
        auto fragShaderCode = VulkanCube::readFile("shader.frag.spv");

        // Create pipeline with loaded shaders
        context.createGraphicsPipeline(vertShaderCode, fragShaderCode);

        createVertexBuffer();
        createIndexBuffer();
        createUniformBuffer();
        descriptorSets = VulkanCube::DescriptorSets::create(context, uniformBuffer, texture );
    }

    void createVertexBuffer() {
        vertexBuffer = VulkanCube::BufferPackage::create(
            context,
            sizeof(vertices[0]) * vertices.size(),
            vk::BufferUsageFlagBits::eVertexBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );

        void* data = context.device->mapMemory(*vertexBuffer.memory, 0, sizeof(vertices[0]) * vertices.size()).value;
        memcpy(data, vertices.data(), sizeof(vertices[0]) * vertices.size());
        context.device->unmapMemory(*vertexBuffer.memory);
    }

    void createIndexBuffer() {
        indexBuffer = VulkanCube::BufferPackage::create(
            context,
            sizeof(indices[0]) * indices.size(),
            vk::BufferUsageFlagBits::eIndexBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );

        void* data = context.device->mapMemory(*indexBuffer.memory, 0, sizeof(indices[0]) * indices.size()).value;
        memcpy(data, indices.data(), sizeof(indices[0]) * indices.size());
        context.device->unmapMemory(*indexBuffer.memory);
    }

    void createUniformBuffer() {
        uniformBuffer = VulkanCube::BufferPackage::create(
            context,
            sizeof(VulkanCube::UniformBufferObject),
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );
    }

    void updateUniformBuffer() {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float>(currentTime - startTime).count();

        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), { 0.0f, 0.0f, 1.0f });
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(
            glm::radians(45.0f),
            context.swapchainExtent.width / (float)context.swapchainExtent.height,
            0.1f,
            10.0f
        );
        ubo.proj[1][1] *= -1;

        void* data = context.device->mapMemory(*uniformBuffer.memory, 0, sizeof(ubo)).value;
        memcpy(data, &ubo, sizeof(ubo));
        context.device->unmapMemory(*uniformBuffer.memory);
    }

    void drawFrame() {
        updateUniformBuffer();

        auto& commandBuffer = commandPool.buffers[context.currentFrame].get();

        vk::Result result;
        uint32_t imageIndex;

        try {
            auto [acquireResult, acquiredImageIndex] = context.device->acquireNextImageKHR(
                *context.swapchain, UINT64_MAX, *context.imageAvailableSemaphores[context.currentFrame], nullptr
            );
            result = acquireResult;
            imageIndex = acquiredImageIndex;
        }
        catch (const vk::OutOfDateKHRError&) {
            recreateSwapchain();
            return;
        }

        if (result == vk::Result::eErrorOutOfDateKHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapchain();
            return;
        }

        commandBuffer.reset();
        commandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

        vk::RenderPassBeginInfo renderPassInfo{
            *context.renderPass,
            *context.swapchainFramebuffers[imageIndex],
            {{0, 0}, context.swapchainExtent},
            1,
            &context.clearValues
        };

        commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *context.graphicsPipeline);
        commandBuffer.bindVertexBuffers(0, { *vertexBuffer.buffer }, { 0 });
        commandBuffer.bindIndexBuffer(*indexBuffer.buffer, 0, vk::IndexType::eUint16);
        commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            *context.pipelineLayout,
            0,
            { *descriptorSets.sets[context.currentFrame] },
            {}
        );
        commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        commandBuffer.endRenderPass();
        commandBuffer.end();

        vk::SubmitInfo submitInfo{
            1,
            &*context.imageAvailableSemaphores[context.currentFrame],
            &vk::PipelineStageFlags{vk::PipelineStageFlagBits::eColorAttachmentOutput},
            1,
            &commandBuffer,
            1,
            &*context.renderFinishedSemaphores[context.currentFrame]
        };

        context.graphicsQueue.submit(submitInfo, *context.inFlightFences[context.currentFrame]);

        try {
            result = context.presentQueue.presentKHR({
                1,
                &*context.renderFinishedSemaphores[context.currentFrame],
                1,
                &*context.swapchain,
                &imageIndex
                });
        }
        catch (const vk::OutOfDateKHRError&) {
            result = vk::Result::eErrorOutOfDateKHR;
        }

        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapchain();
        }
        else if (result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to present swap chain image!");
        }

        context.currentFrame = (context.currentFrame + 1) % VulkanCube::MAX_FRAMES_IN_FLIGHT;
    }

    void recreateSwapchain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        context.device->waitIdle();
        context.recreateSwapchain(window);
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            drawFrame();
        }
        context.device->waitIdle();
    }

    void cleanup() {
        context.device->waitIdle();

        uniformBuffer = {};
        indexBuffer = {};
        vertexBuffer = {};
        texture = {};
        descriptorSets = {};
        commandPool = {};
        context = {};

        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

int main() {
    CubeApp app;
    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}