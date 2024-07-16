//
// Created by gbian on 15/07/2024.
//

#include "vulkrt/App.hpp"
#include "vulkrt/FPSCounter.hpp"

namespace lve {
    DISABLE_WARNINGS_PUSH(26432 26447)
    App::App() noexcept {
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

    void App::createPipelineLayout() {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        if(vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failde to  create pipeline layput!");
        }
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

        if(vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to  create comand buffers");
        }
        for(int i = 0; i < commandBuffers.size(); i++) {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if(vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

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
            vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

            vkCmdEndRenderPass(commandBuffers[i]);
            if(vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) { throw std::runtime_error("failed to record command buffer!"); }
        }
    }

    void App::drawFrame() {
        uint32_t imageIndex;
        auto result = lveSwapChain.acquireNextImage(&imageIndex);
        if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { throw std::runtime_error("failed to acquire swap chain image!"); }

        result = lveSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
        if(result != VK_SUCCESS) { throw std::runtime_error("failed to present swap chain image!"); }
    }
    DISABLE_WARNINGS_POP()

}  // namespace lve
