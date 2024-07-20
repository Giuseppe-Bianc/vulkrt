//
// Created by gbian on 15/07/2024.
//
// NOLINTBEGIN(*-include-cleaner)
#include "vulkrt/App.hpp"

#include <vulkrt/FPSCounter.hpp>

namespace lve {
    DISABLE_WARNINGS_PUSH(26432 26447)
    App::App() noexcept {
        loadGameObjects();
        createPipelineLayout();
        recreateSwapChain();
        createCommandBuffers();
    }

    App::~App() { vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr); }
    DISABLE_WARNINGS_POP()
    void App::run() {
        FPSCounter fps_counter{lveWindow.getGLFWWindow(), WTITILE};
        while(!lveWindow.shouldClose()) {
            fps_counter.frameInTitle();
            // Take care of all GLFW events
            glfwPollEvents();
            drawFrame();
        }
        vkDeviceWaitIdle(lveDevice.device());
    }

    /*void sierpinski(
    std::vector<Model::Vertex> &vertices,
    int depth,
    const Model::Vertex& left,
    const Model::Vertex& right,
    const Model::Vertex& top) {
        if (depth <= 0) {
            vertices.emplace_back(top);
            vertices.emplace_back(right);
            vertices.emplace_back(left);
        } else {
            auto leftTopP = 0.5f * (left.position + top.position);
            auto rightTopP = 0.5f * (right.position + top.position);
            auto leftRightP = 0.5f * (left.position + right.position);
            auto leftTopC = 0.5f * (left.color + top.color);
            auto rightTopC = 0.5f * (right.color + top.color);
            auto leftRightC = 0.5f * (left.color + right.color);
            sierpinski(vertices, depth - 1, left, {leftRightP, leftRightC}, {leftTopP,leftTopC});
            sierpinski(vertices, depth - 1, {leftRightP, leftRightC}, right, {rightTopP, rightTopC});
            sierpinski(vertices, depth - 1, {leftTopP, leftTopC}, {rightTopP, rightTopC}, top);
        }
    }*/

    void App::loadGameObjects() {
        std::vector<Model::Vertex> vertices{
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}}, {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}, {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
        const auto lveModel = MAKE_SHARED(Model, lveDevice, vertices);

        auto triangle = GameObject::createGameObject();
        triangle.model = lveModel;
        triangle.color = {.1f, .8f, .1f};
        triangle.transform2d.translation.x = .2f;
        triangle.transform2d.scale = {2.f, .5f};
        triangle.transform2d.rotation = .25f * glm::two_pi<float>();

        gameObjects.push_back(std::move(triangle));
    }

    void App::createPipelineLayout() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        VK_CHECK(vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout),
                 "failed to  create pipeline layout!");
    }

    void App::recreateSwapChain() {
        auto extent = lveWindow.getExtent();
        while(extent.width == 0 || extent.height == 0) {
            extent = lveWindow.getExtent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(lveDevice.device());

        if(lveSwapChain == nullptr) {
            lveSwapChain = MAKE_UNIQUE(SwapChain, lveDevice, extent);
        } else {
            lveSwapChain = MAKE_UNIQUE(SwapChain, lveDevice, extent, std::move(lveSwapChain));
            if(lveSwapChain->imageCount() != commandBuffers.size()) {
                freeCommandBuffers();
                createCommandBuffers();
            }
        }

        createPipeline();
    }

    void App::createPipeline() {
        assert(lveSwapChain != nullptr && "Cannot create pipeline before swap chain");
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = lveSwapChain->getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        const auto vertPath = Window::calculateRelativePathToSrcRes(curent, "simple_shader.vert.spv").string();
        // TODO: return to .frag.vert
        const auto fragPath = Window::calculateRelativePathToSrcRes(curent, "simple_shader.frag.spv").string();
        lvePipeline = MAKE_UNIQUE(Pipeline, lveDevice, vertPath, fragPath, pipelineConfig);
    }
    DISABLE_WARNINGS_PUSH(26429 26432 26461 26446 26485)
    void App::createCommandBuffers() {
        commandBuffers.resize(lveSwapChain->imageCount());
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = lveDevice.getCommandPool();
        allocInfo.commandBufferCount = C_UI32T(commandBuffers.size());

        VK_CHECK(vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()), "failed to allocate command buffers!");
    }

    void App::freeCommandBuffers() noexcept {
        vkFreeCommandBuffers(lveDevice.device(), lveDevice.getCommandPool(), C_UI32T(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
    }

    void App::recordCommandBuffer(int imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VK_CHECK(vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo), "failed to begin recording command buffer!");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = lveSwapChain->getRenderPass();
        renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(imageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = C_UI32T(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = C_F(lveSwapChain->getSwapChainExtent().width);
        viewport.height = C_F(lveSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        const VkRect2D scissor{{0, 0}, lveSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
        vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

        renderGameObjects(commandBuffers[imageIndex]);

        vkCmdEndRenderPass(commandBuffers[imageIndex]);
        VK_CHECK(vkEndCommandBuffer(commandBuffers[imageIndex]), "failed to record command buffer!");
    }

    void App::renderGameObjects(VkCommandBuffer commandBuffer) {
        lvePipeline->bind(commandBuffer);

        for(auto &obj : gameObjects) {
            obj.transform2d.rotation = glm::mod(obj.transform2d.rotation + 0.01f, glm::two_pi<float>());

            SimplePushConstantData push{};
            push.offset = obj.transform2d.translation;
            push.color = obj.color;
            push.transform = obj.transform2d.mat2();

            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                               sizeof(SimplePushConstantData), &push);
            obj.model->bind(commandBuffer);
            obj.model->draw(commandBuffer);
        }
    }

    void App::drawFrame() {
        uint32_t imageIndex;
        auto result = lveSwapChain->acquireNextImage(&imageIndex);

        if(result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }

        VK_CHECK_SWAPCHAIN(result,"failed to acquire swap chain image!");

        recordCommandBuffer(imageIndex);
        result = lveSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.wasWindowResized()) {
            lveWindow.resetWindowResizedFlag();
            recreateSwapChain();
            return;
        } else if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
    }
    DISABLE_WARNINGS_POP()

}  // namespace lve
   // NOLINTEND(*-include-cleaner)