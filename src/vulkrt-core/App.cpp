//
// Created by gbian on 15/07/2024.
//

#include "vulkrt/App.hpp"
#include "vulkrt/FPSCounter.hpp"

namespace lve {
    DISABLE_WARNINGS_PUSH(26432 26447)
    App::App() noexcept {
        loadModels();
        createPipelineLayout();
        createPipeline();
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
    void App::loadModels() {
        /*std::vector<Model::Vertex> vertices{};
        sierpinski(vertices, 3,  {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}}, {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}, {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}});*/
        std::vector<Model::Vertex> vertices{
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}}, {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}, {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
        lveModel = MAKE_UNIQUE(Model, lveDevice, vertices);
    }

    void App::createPipelineLayout() {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        VK_CHECK(vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout),
                 "failde to  create pipeline layput!");
    }

    void App::createPipeline() {
        auto pipelineConfig = Pipeline::defaultPipelineConfigInfo(lveSwapChain.width(), lveSwapChain.height());
        pipelineConfig.renderPass = lveSwapChain.getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        const auto curent = fs::current_path();
        const auto vertPath = lveWindow.calculateRelativePathToSrcRes(curent, "simple_shader.vert.spv").string();
        const auto fragPath = lveWindow.calculateRelativePathToSrcRes(curent, "simple_shader.frag.spv").string();
        lvePipeline = MAKE_UNIQUE(Pipeline, lveDevice, vertPath, fragPath, pipelineConfig);
    }
    DISABLE_WARNINGS_PUSH(26429 26432 26461 26446 26485)
    void App::createCommandBuffers() {
        commandBuffers.resize(lveSwapChain.imageCount());
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = lveDevice.getCommandPool();
        allocInfo.commandBufferCount = C_UI32T(commandBuffers.size());

        VK_CHECK(vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()), "failed to  create comand buffers");
        for(int i = 0; i < commandBuffers.size(); i++) {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            VK_CHECK(vkBeginCommandBuffer(commandBuffers[i], &beginInfo), "failed to begin recording command buffer!");

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = lveSwapChain.getRenderPass();
            renderPassInfo.framebuffer = lveSwapChain.getFrameBuffer(i);

            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = lveSwapChain.getSwapChainExtent();

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
            clearValues[1].depthStencil = {1.0f, 0};
            renderPassInfo.clearValueCount = C_UI32T(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            lvePipeline->bind(commandBuffers[i]);
            lveModel->bind(commandBuffers[i]);
            lveModel->draw(commandBuffers[i]);

            vkCmdEndRenderPass(commandBuffers[i]);
            VK_CHECK(vkEndCommandBuffer(commandBuffers[i]), "failed to record command buffer!");
        }
    }

    void App::drawFrame() {
        uint32_t imageIndex;
        auto result = lveSwapChain.acquireNextImage(&imageIndex);
        if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { throw std::runtime_error("failed to acquire swap chain image!"); }

        result = lveSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
        VK_CHECK(result, "failed to present swap chain image!");
    }
    DISABLE_WARNINGS_POP()

}  // namespace lve
