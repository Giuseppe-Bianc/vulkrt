#pragma once
// NOLINTBEGIN(*-include-cleaner)
#include "Device.hpp"

namespace lve {

    class SwapChain {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        SwapChain(Device &deviceRef, const VkExtent2D &windowExtent) noexcept;
        SwapChain(Device &deviceRef, const VkExtent2D &windowExtent, std::shared_ptr<SwapChain> previous);
        ~SwapChain();

        SwapChain(const SwapChain &) = delete;
        SwapChain &operator=(const SwapChain &) = delete;

        DISABLE_WARNINGS_PUSH(26446)
        [[nodiscard]] VkFramebuffer getFrameBuffer(int index) const noexcept { return swapChainFramebuffers[index]; }
        [[nodiscard]] VkRenderPass getRenderPass() const noexcept { return renderPass; }
        [[nodiscard]] VkImageView getImageView(int index) const noexcept { return swapChainImageViews[index]; }
        [[nodiscard]] size_t imageCount() const noexcept { return swapChainImages.size(); }
        [[nodiscard]] VkFormat getSwapChainImageFormat() const noexcept { return swapChainImageFormat; }
        [[nodiscard]] VkExtent2D getSwapChainExtent() const noexcept { return swapChainExtent; }
        [[nodiscard]] uint32_t width() const noexcept { return swapChainExtent.width; }
        [[nodiscard]] uint32_t height() const noexcept { return swapChainExtent.height; }

        [[nodiscard]] float extentAspectRatio() const noexcept { return C_F(swapChainExtent.width) / C_F(swapChainExtent.height); }
        [[nodiscard]] VkFormat findDepthFormat() const;

        [[nodiscard]] VkResult acquireNextImage(uint32_t *imageIndex) const noexcept;
        [[nodiscard]] VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);
        DISABLE_WARNINGS_POP()

    private:
        void init();
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
        std::shared_ptr<SwapChain> oldSwapChain;

        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;
        size_t currentFrame = 0;
    };

}  // namespace lve
   // NOLINTEND(*-include-cleaner)