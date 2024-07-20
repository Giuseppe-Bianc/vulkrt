// NOLINTBEGIN(*-include-cleaner)
#include "vulkrt/SwapChain.hpp"
#include <vulkrt/timer/Timer.hpp>

namespace lve {
    DISABLE_WARNINGS_PUSH(26429 26432 26447 26461 26446 26485)
    SwapChain::SwapChain(Device &deviceRef, const VkExtent2D &extent) noexcept : device{deviceRef}, windowExtent{extent} { init(); }

    SwapChain::SwapChain(Device &deviceRef, const VkExtent2D &extent, std::shared_ptr<SwapChain> previous)
      : device{deviceRef}, windowExtent{extent}, oldSwapChain{std::move(previous)} {
        init();
        oldSwapChain = nullptr;
    }

    void SwapChain::init() {
        createSwapChain();
        createImageViews();
        createRenderPass();
        createDepthResources();
        createFramebuffers();
        createSyncObjects();
    }

    SwapChain::~SwapChain() {
        for(auto *imageView : swapChainImageViews) { vkDestroyImageView(device.device(), imageView, nullptr); }
        swapChainImageViews.clear();

        if(swapChain != nullptr) {
            vkDestroySwapchainKHR(device.device(), swapChain, nullptr);
            swapChain = nullptr;
        }

        for(int i = 0; i < depthImages.size(); i++) {
            vkDestroyImageView(device.device(), depthImageViews[i], nullptr);
            vkDestroyImage(device.device(), depthImages[i], nullptr);
            vkFreeMemory(device.device(), depthImageMemorys[i], nullptr);
        }

        for(auto *const framebuffer : swapChainFramebuffers) { vkDestroyFramebuffer(device.device(), framebuffer, nullptr); }

        vkDestroyRenderPass(device.device(), renderPass, nullptr);

        // cleanup synchronization objects
        for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device.device(), renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device.device(), imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device.device(), inFlightFences[i], nullptr);
        }
    }

    VkResult SwapChain::acquireNextImage(uint32_t *imageIndex) const noexcept {
        vkWaitForFences(device.device(), 1, &inFlightFences[currentFrame], VK_TRUE, MAXU64);

        const VkResult result = vkAcquireNextImageKHR(device.device(), swapChain, MAXU64,
                                                      imageAvailableSemaphores[currentFrame],  // must be a not signaled semaphore
                                                      VK_NULL_HANDLE, imageIndex);

        return result;
    }

    VkResult SwapChain::submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex) {
        if(imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(device.device(), 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
        }
        imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        const std::vector<VkSemaphore> waitSemaphores = {imageAvailableSemaphores[currentFrame]};
        const std::vector<VkPipelineStageFlags> waitStages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.pWaitDstStageMask = waitStages.data();

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffers;

        const std::vector<VkSemaphore> signalSemaphores = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores.data();

        vkResetFences(device.device(), 1, &inFlightFences[currentFrame]);
        VK_CHECK(vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]),
                 "failed to submit draw command buffer!");

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores.data();

        std::vector<VkSwapchainKHR> swapChains = {swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains.data();

        presentInfo.pImageIndices = imageIndex;

        const auto result = vkQueuePresentKHR(device.presentQueue(), &presentInfo);

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        return result;
    }

    void SwapChain::createSwapChain() {
        const SwapChainSupportDetails swapChainSupport = device.getSwapChainSupport();

        const VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        const VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        const VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = device.surface();

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        const QueueFamilyIndices indices = device.findPhysicalQueueFamilies();
        std::vector<uint32_t> queueFamilyIndices = {indices.graphicsFamily, indices.presentFamily};

        if(indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;      // Optional
            createInfo.pQueueFamilyIndices = nullptr;  // Optional
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = oldSwapChain == nullptr ? VK_NULL_HANDLE : oldSwapChain->swapChain;

        VK_CHECK(vkCreateSwapchainKHR(device.device(), &createInfo, nullptr, &swapChain), "failed to create swap chain!");

        // we only specified a minimum number of images in the swap chain, so the implementation is
        // allowed to create a swap chain with more. That's why we'll first query the final number of
        // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
        // retrieve the handles.
        vkGetSwapchainImagesKHR(device.device(), swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device.device(), swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void SwapChain::createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());
        for(const auto [i, image] : std::views::enumerate(swapChainImages)) {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = swapChainImageFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            VK_CHECK(vkCreateImageView(device.device(), &viewInfo, nullptr, &swapChainImageViews[i]),
                     "failed to create texture image view!");
        }
    }

    void SwapChain::createRenderPass() {
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = getSwapChainImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency = {};

        dependency.dstSubpass = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.srcAccessMask = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = C_UI32T(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        VK_CHECK(vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderPass), "failed to create render pass!");
    }

    void SwapChain::createFramebuffers() {
#ifdef INDEPTH
        const vnd::AutoTimer timer{"createFramebuffers", vnd::Timer::Big};
#endif
        const auto imagectn = imageCount();
        swapChainFramebuffers.resize(imagectn);
        for(size_t i = 0; i < imagectn; i++) {
            std::array<VkImageView, 2> attachments = {swapChainImageViews[i], depthImageViews[i]};

            const VkExtent2D swapChainExtentm = getSwapChainExtent();
            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = C_UI32T(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtentm.width;
            framebufferInfo.height = swapChainExtentm.height;
            framebufferInfo.layers = 1;

            VK_CHECK(vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]),
                     "failed to create framebuffer!");
        }
    }

    void SwapChain::createDepthResources() {
        const VkFormat depthFormat = findDepthFormat();
        const VkExtent2D swapChainExtentm = getSwapChainExtent();

        depthImages.resize(imageCount());
        depthImageMemorys.resize(imageCount());
        depthImageViews.resize(imageCount());

        for(int i = 0; i < depthImages.size(); i++) {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = swapChainExtentm.width;
            imageInfo.extent.height = swapChainExtentm.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = depthFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;

            device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImages[i], depthImageMemorys[i]);

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = depthImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = depthFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            VK_CHECK(vkCreateImageView(device.device(), &viewInfo, nullptr, &depthImageViews[i]), "failed to create texture image view!");
        }
    }

    void SwapChain::createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VK_CHECK_SYNC_OBJECTS(vkCreateSemaphore(device.device(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]),
                                  vkCreateSemaphore(device.device(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]),
                                  vkCreateFence(device.device(), &fenceInfo, nullptr, &inFlightFences[i]),
                                  "failed to create synchronization objects for a frame!");
        }
    }

    VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) const noexcept {
#ifdef INDEPTH
        const vnd::AutoTimer timer{"chooseSwapSurfaceFormat", vnd::Timer::Big};
#endif
        for(const auto &availableFormat : availableFormats) {
            if(availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) const {
#ifdef INDEPTH
        const vnd::AutoTimer timer{"chooseSwapPresentMode", vnd::Timer::Big};
#endif
        // Prefer VK_PRESENT_MODE_MAILBOX_KHR if available
        if(std::ranges::find(availablePresentModes, VK_PRESENT_MODE_MAILBOX_KHR) != availablePresentModes.end()) {
            LINFO("Present mode: Mailbox");
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }

        // Fallback to VK_PRESENT_MODE_IMMEDIATE_KHR if needed
        // This code is commented out in your example, but if you want to enable it, you can use the following:
        // if (std::ranges::find(availablePresentModes, VK_PRESENT_MODE_IMMEDIATE_KHR) != availablePresentModes.end()) {
        //     LINFO("Present mode: Immediate");
        //     return VK_PRESENT_MODE_IMMEDIATE_KHR;
        // }

        // Default to VK_PRESENT_MODE_FIFO_KHR (V-Sync)
        LINFO("Present mode: V-Sync");
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const noexcept {
#ifdef INDEPTH
        const vnd::AutoTimer timer{"chooseSwapExtent", vnd::Timer::Big};
#endif
        if(capabilities.currentExtent.width != MAXU32) [[likely]] {
            return capabilities.currentExtent;
        } else [[unlikely]] {
            VkExtent2D actualExtent = windowExtent;
            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
            return actualExtent;
        }
    }

    VkFormat SwapChain::findDepthFormat() const {
#ifdef INDEPTH
#endif
        const vnd::AutoTimer timer{"findDepthFormat", vnd::Timer::Big};
        return device.findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                          VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }
    DISABLE_WARNINGS_POP()
}  // namespace lve

// NOLINTEND(*-include-cleaner)
