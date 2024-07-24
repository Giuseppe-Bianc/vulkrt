//
// Created by gbian on 20/07/2024.
//

#pragma once
#include "Device.hpp"
#include "SwapChain.hpp"
#include "Window.hpp"

namespace lve {
    class Renderer {
    public:
        Renderer(Window &window, Device &device) noexcept;
        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        [[nodiscard]] VkRenderPass getSwapChainRenderPass() const noexcept { return lveSwapChain->getRenderPass(); }
        [[nodiscard]] float getAspectRatio() const noexcept { return lveSwapChain->extentAspectRatio(); }
        [[nodiscard]] bool isFrameInProgress() const noexcept { return isFrameStarted; }

        DISABLE_WARNINGS_PUSH(26446)
        [[nodiscard]] VkCommandBuffer getCurrentCommandBuffer() const noexcept {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return commandBuffers[currentFrameIndex];
        }
        DISABLE_WARNINGS_POP()

        [[nodiscard]] int getFrameIndex() const noexcept {
            assert(isFrameStarted && "Cannot get frame index when frame not in progress");
            return currentFrameIndex;
        }

        [[nodiscard]] VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer) noexcept;
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer) noexcept;

    private:
        void createCommandBuffers();
        void freeCommandBuffers() noexcept;
        void recreateSwapChain();

        Window &lveWindow;
        Device &lveDevice;
        std::unique_ptr<SwapChain> lveSwapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex;
        bool isFrameStarted;
    };

}  // namespace lve
// NOLINTEND(*-include-cleaner)
