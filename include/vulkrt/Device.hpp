// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "Window.hpp"

namespace lve {

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
        static void printDetails(const SwapChainSupportDetails &swapCainsDetails);
    };

    struct QueueFamilyIndices {
        uint32_t graphicsFamily{};
        uint32_t presentFamily{};
        bool graphicsFamilyHasValue = false;
        bool presentFamilyHasValue = false;
        [[nodiscard]] bool isComplete() const noexcept { return graphicsFamilyHasValue && presentFamilyHasValue; }
    };

    class Device {
    public:
#ifdef NDEBUG
        static inline constexpr bool enableValidationLayers = false;
#else
        static inline constexpr bool enableValidationLayers = true;
#endif

        explicit Device(Window &window) noexcept;
        ~Device();

        // Not copyable or movable
        Device(const Device &) = delete;
        Device &operator=(const Device &) = delete;
        Device(Device &&) = delete;
        Device &operator=(Device &&) = delete;

        [[nodiscard]] VkCommandPool getCommandPool() const noexcept { return commandPool; }
        [[nodiscard]] VkDevice device() const noexcept { return device_; }
        [[nodiscard]] VkSurfaceKHR surface() const noexcept { return surface_; }
        [[nodiscard]] VkQueue graphicsQueue() const noexcept { return graphicsQueue_; }
        [[nodiscard]] VkQueue presentQueue() const noexcept { return presentQueue_; }

        [[nodiscard]] SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
        [[nodiscard]] uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        [[nodiscard]] QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
        [[nodiscard]] VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                                   VkFormatFeatureFlags features);

        // Buffer Helper Functions
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer,
                          VkDeviceMemory &bufferMemory);
        [[nodiscard]] VkCommandBuffer beginSingleTimeCommands() noexcept;
        void endSingleTimeCommands(VkCommandBuffer commandBuffer) noexcept;
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) noexcept;
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) noexcept;

        void createImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties, VkImage &image,
                                 VkDeviceMemory &imageMemory);

        VkPhysicalDeviceProperties properties;

    private:
        void createInstance();
        void setupDebugMessenger();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();

        // helper functions
        [[nodiscard]] bool isDeviceSuitable(VkPhysicalDevice device);
        [[nodiscard]] std::vector<const char *> getRequiredExtensions() const;
        [[nodiscard]] bool checkValidationLayerSupport() const;
        [[nodiscard]] QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) const noexcept;
        void hasGflwRequiredInstanceExtensions() const;
        [[nodiscard]] bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;
        [[nodiscard]] SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        Window &window;
        VkCommandPool commandPool;

        VkDevice device_;
        VkSurfaceKHR surface_;
        VkQueue graphicsQueue_;
        VkQueue presentQueue_;

        const std::vector<const char *> validationLayers{"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char *> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    };

}  // namespace lve
   // NOLINTEND(*-include-cleaner)
