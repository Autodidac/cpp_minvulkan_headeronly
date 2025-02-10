#ifdef _WIN32
    #include "..\\VulkanStaticLib1\\framework.h"
    #include "..\\VulkanStaticLib1\\include\\vulkanbuffers.hpp"
    #include "..\\VulkanStaticLib1\\include\\vulkancore.hpp"
    #include "..\\VulkanStaticLib1\\include\\vulkanpipeline.hpp"
    #include "..\\VulkanStaticLib1\\include\\vulkantextures.hpp"
    #include "..\\VulkanStaticLib1\\include\\vulkancommands.hpp"
    #include "..\\VulkanStaticLib1\\include\\vulkandescriptors.hpp"
#else
    #include "../VulkanStaticLib1/include/vulkanbuffers.hpp"
    #include "../VulkanStaticLib1/include/vulkancore.hpp"
    #include "../VulkanStaticLib1/include/vulkanpipeline.hpp"
    #include "../VulkanStaticLib1/include/vulkantextures.hpp"
    #include "../VulkanStaticLib1/include/vulkancommands.hpp"
    #include "../VulkanStaticLib1/include/vulkandescriptors.hpp"
#endif

#include <GLFW/glfw3.h>

#include <chrono>
#include <iostream>
#include <vector>
#include <cstring>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

// --- Cube vertex and index data (unchanged) ---
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

// --- CubeApp class ---
class CubeApp {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window = nullptr;
    VulkanCube::Context context;
    VulkanCube::CommandPool commandPool;
    VulkanCube::Texture texture;
    VulkanCube::BufferPackage vertexBuffer;
    VulkanCube::BufferPackage indexBuffer;
    VulkanCube::BufferPackage uniformBuffer;
    VulkanCube::DescriptorSets descriptorSets;
    VulkanCube::UniformBufferObject ubo{};
    bool framebufferResized = false;
    // Use std::vector<char> for shader code
    std::vector<char> vertShaderCode;
    std::vector<char> fragShaderCode;

    void initWindow() {
        if (!glfwInit())
            throw std::runtime_error("Failed to initialize GLFW");
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(800, 600, "Vulkan Cube", nullptr, nullptr);
        if (!window)
            throw std::runtime_error("Failed to create GLFW window");
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    static void framebufferResizeCallback(GLFWwindow* window, int, int) {
        auto app = reinterpret_cast<CubeApp*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    void initVulkan() {
        context = VulkanCube::Context::createWindow(window, true);
        commandPool = VulkanCube::CommandPool::create(context, 2);
        texture = VulkanCube::Texture::loadFromFile(context, commandPool, "texture.jpg");

        // Load shaders into vectors of char:
        try {
            vertShaderCode = VulkanCube::readFile("shader.vert.spv");
            fragShaderCode = VulkanCube::readFile("shader.frag.spv");
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to load shaders: " + std::string(e.what()));
        }

        // Create the graphics pipeline from the shader code.
        auto graphicsPipeline = VulkanCube::GraphicsPipeline::createPipeline(context, vertShaderCode, fragShaderCode);
        // Assign pipeline objects to your context (this depends on your design)
        context.graphicsPipeline = std::move(graphicsPipeline.pipeline);
        context.pipelineLayout   = std::move(graphicsPipeline.layout);
        context.renderPass       = std::move(graphicsPipeline.renderPass);

        createVertexBuffer();
        createIndexBuffer();
        createUniformBuffer();
        descriptorSets = VulkanCube::DescriptorSets::create(context, uniformBuffer, texture.view.get());
    }

    void createVertexBuffer() {
        vertexBuffer = VulkanCube::BufferPackage::create(
            context,
            sizeof(vertices[0]) * vertices.size(),
            vk::BufferUsageFlagBits::eVertexBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );
        void* data = context.device->mapMemory(*vertexBuffer.memory, 0, sizeof(vertices[0]) * vertices.size()).value;
        std::memcpy(data, vertices.data(), sizeof(vertices[0]) * vertices.size());
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
        std::memcpy(data, indices.data(), sizeof(indices[0]) * indices.size());
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
        ubo.view  = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f),   glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj  = glm::perspective(
            glm::radians(45.0f),
            static_cast<float>(context.swapchainExtent.width) / static_cast<float>(context.swapchainExtent.height),
            0.1f,
            10.0f
        );
        ubo.proj[1][1] *= -1;

        void* data = context.device->mapMemory(*uniformBuffer.memory, 0, sizeof(ubo)).value;
        std::memcpy(data, &ubo, sizeof(ubo));
        context.device->unmapMemory(*uniformBuffer.memory);
    }

// Revised drawFrame() without exception types that aren’t defined:
void drawFrame() {
    updateUniformBuffer();

    auto& commandBuffer = commandPool.buffers[context.currentFrame].get();
    vk::Result result;
    uint32_t imageIndex;
    
    // Use the non-throwing version of acquireNextImageKHR:
    result = context.device->acquireNextImageKHR(
                *context.swapchain, UINT64_MAX,
                *context.imageAvailableSemaphores[context.currentFrame],
                nullptr, &imageIndex);
                
    if (result == vk::Result::eErrorOutOfDateKHR ||
        result == vk::Result::eSuboptimalKHR ||
        framebufferResized) {
        recreateSwapchain();
        framebufferResized = false;
        return;
    }
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to acquire swapchain image!");
    }

    (void) context.device->waitForFences({ *context.inFlightFences[context.currentFrame] }, VK_TRUE, UINT64_MAX);
    (void) context.device->resetFences({ *context.inFlightFences[context.currentFrame] });

    commandBuffer.reset();
    (void) commandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

    // Define a local clear value instead of using context.clearValues:
    vk::ClearValue clearValue;
    clearValue.color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});
    
    vk::RenderPassBeginInfo renderPassInfo(
        *context.renderPass,
        *context.swapchainFramebuffers[imageIndex],
        vk::Rect2D({0, 0}, context.swapchainExtent),
        1,
        &clearValue
    );
    
    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *context.graphicsPipeline);
    commandBuffer.bindVertexBuffers(0, { *vertexBuffer.buffer }, { 0 });
    commandBuffer.bindIndexBuffer(*indexBuffer.buffer, 0, vk::IndexType::eUint16);
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        *context.pipelineLayout,
        0,
        { descriptorSets.sets[context.currentFrame] },
        {}
    );
    commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    commandBuffer.endRenderPass();
    (void) commandBuffer.end();

    // Use a local variable for the wait stage flag.
    vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submitInfo{
        1,
        &*context.imageAvailableSemaphores[context.currentFrame],
        &waitStage,
        1,
        &commandBuffer,
        1,
        &*context.renderFinishedSemaphores[context.currentFrame]
    };

    (void) context.graphicsQueue.submit(submitInfo, *context.inFlightFences[context.currentFrame]);

    vk::PresentInfoKHR presentInfo{
        1,
        &*context.renderFinishedSemaphores[context.currentFrame],
        1,
        &*context.swapchain,
        &imageIndex
    };

    result = context.presentQueue.presentKHR(presentInfo);
    if (result == vk::Result::eErrorOutOfDateKHR ||
        result == vk::Result::eSuboptimalKHR ||
        framebufferResized) {
        recreateSwapchain();
        framebufferResized = false;
    } else if (result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    context.currentFrame = (context.currentFrame + 1) % VulkanCube::Context::MAX_FRAMES_IN_FLIGHT;
}

void recreateSwapchain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vk::Result result = context.device->waitIdle();
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to wait for device idle during swapchain recreation!");
    }

    std::cout << "Recreating swapchain..." << std::endl;
    // TODO: Insert your actual swapchain cleanup and recreation logic here.
}

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            drawFrame();
        }
        (void) context.device->waitIdle();
    }

    void cleanup() {
        (void) context.device->waitIdle();

        uniformBuffer = {};
        indexBuffer   = {};
        vertexBuffer  = {};
        texture       = {};
        descriptorSets = {};
        commandPool   = {};
        context       = {};

        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

int main() {
    CubeApp app;
    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
