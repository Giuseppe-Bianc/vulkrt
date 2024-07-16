#pragma once

#include "Device.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

namespace lve {

    class SwapChain {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        SwapChain(Device &deviceRef, VkExtent2D windowExtent) noexcept;
        ~SwapChain();

        SwapChain(const SwapChain &) = delete;
        void operator=(const SwapChain &) = delete;

        DISABLE_WARNINGS_PUSH(26446)
        [[nodiscard]] VkFramebuffer getFrameBuffer(int index) noexcept { return swapChainFramebuffers[index]; }
        [[nodiscard]] VkRenderPass getRenderPass() noexcept { return renderPass; }
        [[nodiscard]] VkImageView getImageView(int index) noexcept { return swapChainImageViews[index]; }
        [[nodiscard]] size_t imageCount() noexcept { return swapChainImages.size(); }
        [[nodiscard]] VkFormat getSwapChainImageFormat() noexcept { return swapChainImageFormat; }
        [[nodiscard]] VkExtent2D getSwapChainExtent() noexcept { return swapChainExtent; }
        [[nodiscard]] uint32_t width() noexcept { return swapChainExtent.width; }
        [[nodiscard]] uint32_t height() noexcept { return swapChainExtent.height; }

        [[nodiscard]] float extentAspectRatio() noexcept { return C_F(swapChainExtent.width) / C_F(swapChainExtent.height); }
        [[nodiscard]] VkFormat findDepthFormat();

        [[nodiscard]] VkResult acquireNextImage(uint32_t *imageIndex) noexcept;
        [[nodiscard]] VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);
        DISABLE_WARNINGS_POP()

    private:
        void createSwapChain();
        void createImageViews();
        void createDepthResources();
        void createRenderPass();
        void createFramebuffers();
        void createSyncObjects();

        // Helper functions
        [[nodiscard]] VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) const noexcept;
        [[nodiscard]] VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) const;
        [[nodiscard]] VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const noexcept;

        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;

        std::vector<VkFramebuffer> swapChainFramebuffers;
        VkRenderPass renderPass;

        std::vector<VkImage> depthImages;
        std::vector<VkDeviceMemory> depthImageMemorys;
        std::vector<VkImageView> depthImageViews;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;

        Device &device;
        VkExtent2D windowExtent;

        VkSwapchainKHR swapChain;

        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;
        size_t currentFrame = 0;
    };

}  // namespace lve
