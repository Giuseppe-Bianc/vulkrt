//
// Created by gbian on 20/07/2024.
//
// NOLINTBEGIN(*-include-cleaner)

#include "vulkrt/Renderer.hpp"
namespace lve {
    DISABLE_WARNINGS_PUSH(26432 26447)
    Renderer::Renderer(Window &window, Device &device) noexcept : lveWindow{window}, lveDevice{device} {
        recreateSwapChain();
        createCommandBuffers();
    }

    Renderer::~Renderer() { freeCommandBuffers(); }
    DISABLE_WARNINGS_POP()

    void Renderer::recreateSwapChain() {
        auto extent = lveWindow.getExtent();
        while(extent.width == 0 || extent.height == 0) {
            extent = lveWindow.getExtent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(lveDevice.device());

        if(lveSwapChain == nullptr) {
            lveSwapChain = MAKE_UNIQUE(SwapChain, lveDevice, extent);
        } else {
            std::shared_ptr<SwapChain> oldSwapChain = std::move(lveSwapChain);
            lveSwapChain = MAKE_UNIQUE(SwapChain, lveDevice, extent, std::move(lveSwapChain));

            if(!oldSwapChain->compareSwapFormats(*lveSwapChain.get())) {
                throw std::runtime_error("Swap chain image(or depth) format has changed!");
            }
        }
    }

    void Renderer::createCommandBuffers() {
        commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        const VkCommandBufferAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = lveDevice.getCommandPool(),
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = C_UI32T(commandBuffers.size()),
        };

        VK_CHECK(vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()), "failed to allocate command buffers!");
    }

    void Renderer::freeCommandBuffers() noexcept {
        vkFreeCommandBuffers(lveDevice.device(), lveDevice.getCommandPool(), C_UI32T(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
    }

    VkCommandBuffer Renderer::beginFrame() {
        assert(!isFrameStarted && "Can't call beginFrame while already in progress");

        const auto result = lveSwapChain->acquireNextImage(&currentImageIndex);
        if(result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return nullptr;
        }

        VK_CHECK_SWAPCHAIN(result, "failed to acquire swap chain image!");

        isFrameStarted = true;

        const auto commandBuffer = getCurrentCommandBuffer();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo), "failed to begin recording command buffer!");
        return commandBuffer;
    }

    void Renderer::endFrame() {
        assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
        const auto commandBuffer = getCurrentCommandBuffer();
        VK_CHECK(vkEndCommandBuffer(commandBuffer), "failed to record command buffer!");

        const auto result = lveSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.wasWindowResized()) {
            lveWindow.resetWindowResizedFlag();
            recreateSwapChain();
        } else if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        isFrameStarted = false;
        currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
    }
    DISABLE_WARNINGS_PUSH(26446)
    void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) noexcept {
        assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

        const auto swpextent = lveSwapChain->getSwapChainExtent();
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01F, 0.01F, 0.01F, 1.0F};  // NOLINT(*-pro-type-union-access)
        clearValues[1].depthStencil = {1.0F, 0};             // NOLINT(*-pro-type-union-access)
        const VkRect2D renderArea{{0, 0}, swpextent};
        const VkRenderPassBeginInfo renderPassInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = lveSwapChain->getRenderPass(),
            .framebuffer = lveSwapChain->getFrameBuffer(C_I(currentImageIndex)),
            .renderArea = renderArea,
            .clearValueCount = C_UI32T(clearValues.size()),
            .pClearValues = clearValues.data(),
        };

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        const VkViewport viewport{
            .x = 0.0f,
            .y = 0.0f,
            .width = C_F(swpextent.width),
            .height = C_F(swpextent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        const VkRect2D scissor{renderArea};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }
    DISABLE_WARNINGS_POP()

    void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) noexcept {
        assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");
        vkCmdEndRenderPass(commandBuffer);
    }
}  // namespace lve
   // NOLINTEND(*-include-cleaner)
