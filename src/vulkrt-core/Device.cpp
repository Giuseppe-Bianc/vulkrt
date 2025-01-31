// NOLINTBEGIN(*-include-cleaner)
#include "vulkrt/Device.hpp"
#include "vulkrt/VlukanLogInfoCallback.hpp"
#include "vulkrt/timer/Timer.hpp"
namespace lve {

    DISABLE_WARNINGS_PUSH(26485 26481 26446 26482)

    void SwapChainSupportDetails::printDetails(const SwapChainSupportDetails &det) {
        LINFO("Swapchain:");
        LINFO("ImgCnt: {}-{}", det.capabilities.minImageCount, det.capabilities.maxImageCount);
        LINFO("CurExt: {}x{}", det.capabilities.currentExtent.width, det.capabilities.currentExtent.height);
        LINFO("ExtRng: {}x{}-{}x{}", det.capabilities.minImageExtent.width, det.capabilities.minImageExtent.height,
              det.capabilities.maxImageExtent.width, det.capabilities.maxImageExtent.height);
        LINFO("ImgLay: {}", det.capabilities.maxImageArrayLayers);
        LINFO("Trnsfm: {}", string_VkSurfaceTransformFlagsKHR(det.capabilities.supportedTransforms));
        LINFO("CurTrns: {}", string_VkSurfaceTransformFlagBitsKHR(det.capabilities.currentTransform));
        LINFO("CmpAlpha: {}", string_VkCompositeAlphaFlagsKHR(det.capabilities.supportedCompositeAlpha));
        LINFO("Usage: {}", string_VkImageUsageFlags(det.capabilities.supportedUsageFlags));

        // Print formats
        LINFO("Available Surface Formats: {}", det.formats.size());
        for(const auto &format : det.formats) {
            LINFO("\tFormat: {}, Color Space: {}", string_VkFormat(format.format), string_VkColorSpaceKHR(format.colorSpace));
        }

        // Print present modes
        LINFO("Available Present Modes: {}", det.presentModes.size());
        for(const auto &presentMode : det.presentModes) { LINFO("\tPresent Mode: {}", string_VkPresentModeKHR(presentMode)); }
    }

    [[nodiscard]] inline static constexpr std::string_view debugCallbackString(VkDebugUtilsMessageTypeFlagsEXT messageType) noexcept {
        switch(messageType) {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            return "[General] ";
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            return "[Validation] ";
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            return "[Performance] ";
        default:
            return "";
        }
    }

    inline static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                               VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                               const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                               [[maybe_unused]] void *pUserData) {
        // Determine the message type
        const std::string_view type = debugCallbackString(messageType);

        // Format and log the message
        const auto msg = FORMAT("{}Message ID: {}({}): {}", type, pCallbackData->pMessageIdName ? pCallbackData->pMessageIdName : "Unknown",
                                pCallbackData->messageIdNumber, pCallbackData->pMessage);

        switch(messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            LTRACE(msg);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            LINFO(msg);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            LWARN(msg);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            LERROR(msg);
            break;
        default:
            LDEBUG(msg);
            break;
        }

        logDebugValidationLayerInfo(pCallbackData, messageSeverity);

        return VK_FALSE;
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger) noexcept {
        // NOLINT
        auto func = std::bit_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
        if(func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks *pAllocator) noexcept {
        // NOLINT
        auto func = std::bit_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        if(func != nullptr) { func(instance, debugMessenger, pAllocator); }
    }

    // class member functions
    DISABLE_WARNINGS_PUSH(26432 26447)
    Device::Device(Window &window) noexcept : window{window} {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createCommandPool();
    }

    Device::~Device() {
        vkDestroyCommandPool(device_, commandPool, nullptr);
        vkDestroyDevice(device_, nullptr);

        if(enableValidationLayers) { DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr); }

        vkDestroySurfaceKHR(instance, surface_, nullptr);
        vkDestroyInstance(instance, nullptr);
    }
    DISABLE_WARNINGS_POP()

    void Device::createInstance() {
        DISABLE_WARNINGS_PUSH(4127)
        if(enableValidationLayers && !checkValidationLayerSupport()) [[unlikely]] {
            throw std::runtime_error("validation layers requested, but not available!");
        }
        DISABLE_WARNINGS_POP()

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "LittleVulkanEngine App";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        const auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = NC_UI32T(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if(enableValidationLayers) {
            createInfo.enabledLayerCount = NC_UI32T(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance), "failed to create instance!");

        hasGflwRequiredInstanceExtensions();
    }

    void Device::pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if(deviceCount == 0) { throw std::runtime_error("failed to find GPUs with Vulkan support!"); }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        physicalDevice = *std::ranges::find_if(devices, [this](const VkPhysicalDevice &device) { return isDeviceSuitable(device); });

        if(physicalDevice == VK_NULL_HANDLE) [[unlikely]] { throw std::runtime_error("failed to find a suitable GPU!"); }
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);
        LINFO("Dev count: {}", deviceCount);
        LINFO("API Ver: {}", properties.apiVersion);
        LINFO("Drv Ver: {}", properties.driverVersion);
        LINFO("Vendor ID: {}", properties.vendorID);
        LINFO("Phys Dev ID: {}", properties.deviceID);
        LINFO("Phys Dev Name: phys dev{}", properties.deviceName);
    }

    void Device::createLogicalDevice() {
        const QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        const std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

        constexpr float queuePriority = 1.0f;
        for(const uint32_t queueFamily : uniqueQueueFamilies) {
            queueCreateInfos.emplace_back(VkDeviceQueueCreateInfo{.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                                                  .queueFamilyIndex = queueFamily,
                                                                  .queueCount = 1,
                                                                  .pQueuePriorities = &queuePriority});
        }

        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = NC_UI32T(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = NC_UI32T(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        // might not really be necessary anymore because device specific validation layers
        // have been deprecated
#ifdef NDEBUG
        if(enableValidationLayers) [[unlikely]] {
            createInfo.enabledLayerCount = NC_UI32T(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else [[likely]] {
            createInfo.enabledLayerCount = 0;
        }
#else
        if(enableValidationLayers) [[likely]] {
            createInfo.enabledLayerCount = NC_UI32T(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else [[unlikely]] {
            createInfo.enabledLayerCount = 0;
        }
#endif

        VK_CHECK(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device_), "failed to create logical device!");

        vkGetDeviceQueue(device_, indices.graphicsFamily, 0, &graphicsQueue_);
        vkGetDeviceQueue(device_, indices.presentFamily, 0, &presentQueue_);
    }

    void Device::createCommandPool() {
        const QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

        // NOLINTBEGIN(*-signed-bitwise)
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        // NOLINTEND(*-signed-bitwise)

        VK_CHECK(vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool), "failed to create command pool!");
    }

    void Device::createSurface() { window.createWindowSurface(instance, &surface_); }

    bool Device::isDeviceSuitable(VkPhysicalDevice device) {
        const QueueFamilyIndices indices = findQueueFamilies(device);

        const bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if(extensionsSupported) {
            const SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            SwapChainSupportDetails::printDetails(swapChainSupport);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    void Device::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) const noexcept {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr;  // Optional
    }

    void Device::setupDebugMessenger() {
        if(!enableValidationLayers) { return; }
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);
        VK_CHECK(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger), "failed to set up debug messenger!");
    }

    bool Device::checkValidationLayerSupport() const {
#ifdef INDEPTH
        vnd::AutoTimer t{"checkValidationLayerSupport", vnd::Timer::Big};
#endif
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        return std::ranges::all_of(validationLayers, [&availableLayers](const char *const &layerName) noexcept {
            return std::ranges::any_of(availableLayers, [layerName](const VkLayerProperties &layerProperties) noexcept {
                return strcmp(layerName, layerProperties.layerName) == 0;
            });
        });
    }

    std::vector<const char *> Device::getRequiredExtensions() const {
        uint32_t glfwExtensionCount = 0;
        const auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensionCount + glfwExtensions);

        if(enableValidationLayers) { extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); }

        return extensions;
    }

    void Device::hasGflwRequiredInstanceExtensions() const {
#ifdef INDEPTH
        vnd::AutoTimer t{"hasGflwRequiredInstanceExtensions", vnd::Timer::Big};
#endif
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        LINFO("available extensions:");
        std::unordered_set<std::string_view> available;
        for(const auto &[extensionName, specVersion] : extensions) {
            LINFO("\t{0}", extensionName);
            available.emplace(extensionName);
        }

        LINFO("required extensions:");
        for(const auto requiredExtensions = getRequiredExtensions(); const auto &required : requiredExtensions) {
            LINFO("\t{0}", required);
            if(!available.contains(required)) [[unlikely]] { throw std::runtime_error("Missing required glfw extension"); }
        }
    }

    bool Device::checkDeviceExtensionSupport(VkPhysicalDevice device) const {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string_view> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for(const auto &[extensionName, specVersion] : availableExtensions) { requiredExtensions.erase(extensionName); }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        for(const auto &[i, queueFamily] : queueFamilies | std::views::enumerate) {
            if(queueFamily.queueCount > 0 && C_BOOL(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {  // NOLINT(*-signed-bitwise)
                indices.graphicsFamily = C_UI32T(i);
                indices.graphicsFamilyHasValue = true;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, C_UI32T(i), surface_, &presentSupport);
            if(queueFamily.queueCount > 0 && C_BOOL(presentSupport)) {
                indices.presentFamily = C_UI32T(i);
                indices.presentFamilyHasValue = true;
            }
            if(indices.isComplete()) { break; }
        }

        return indices;
    }

    SwapChainSupportDetails Device::querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

        uint32_t formatCount{};
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);

        if(formatCount != 0) [[likely]] {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount{};
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);

        if(presentModeCount != 0) [[likely]] {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, details.presentModes.data());
        }
        return details;
    }

    /*static constexpr bool matchesFeatures(const VkFormatFeatureFlags tilingFeatures, const VkFormatFeatureFlags features) noexcept {
        return (tilingFeatures & features) == features;
    }*/

    VkFormat Device::findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        for(const VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            bool supported = false;

            switch(tiling) {
            case VK_IMAGE_TILING_LINEAR:
                supported = (props.linearTilingFeatures & features) == features;
                break;
            case VK_IMAGE_TILING_OPTIMAL:
                supported = (props.optimalTilingFeatures & features) == features;
                break;
            default:
                break;
            }

            if(supported) { return format; }
        }

        throw std::runtime_error("Failed to find supported format!");
    }

    uint32_t Device::findMemoryType(uint32_t typeFilter, const VkMemoryPropertyFlags &mproperties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
        const std::bitset<32> typeBits(typeFilter);
        for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if(typeBits.test(i) && (memProperties.memoryTypes[i].propertyFlags & mproperties) == mproperties) { return i; }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    void Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mproperties, VkBuffer &buffer,
                              VkDeviceMemory &bufferMemory) {
        const VkBufferCreateInfo bufferInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = size, .usage = usage, .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

        VK_CHECK(vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer), "failed to create buffer!");

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);

        const VkMemoryAllocateInfo allocInfo = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                                                .allocationSize = memRequirements.size,
                                                .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, mproperties)};

        VK_CHECK(vkAllocateMemory(device_, &allocInfo, nullptr, &bufferMemory), "failed to allocate vertex buffer memory!");

        vkBindBufferMemory(device_, buffer, bufferMemory, 0);
    }

    VkCommandBuffer Device::beginSingleTimeCommands() noexcept {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

    void Device::endSingleTimeCommands(VkCommandBuffer commandBuffer) noexcept {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue_);

        vkFreeCommandBuffers(device_, commandPool, 1, &commandBuffer);
    }

    void Device::copyBuffer(const VkBuffer &srcBuffer, const VkBuffer &dstBuffer, VkDeviceSize size) noexcept {
        const VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        const VkBufferCopy copyRegion = {.srcOffset = 0, .dstOffset = 0, .size = size};

        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }

    void Device::copyBufferToImage(const VkBuffer &buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) noexcept {
        const VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        const VkBufferImageCopy region = {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = layerCount},
            .imageOffset = {0, 0, 0},
            .imageExtent = {width, height, 1}};

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        endSingleTimeCommands(commandBuffer);
    }

    void Device::createImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags mproperties, VkImage &image,
                                     VkDeviceMemory &imageMemory) {
        VK_CHECK(vkCreateImage(device_, &imageInfo, nullptr, &image), "failed to create image!");

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device_, image, &memRequirements);

        const VkMemoryAllocateInfo allocInfo = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                                                .allocationSize = memRequirements.size,
                                                .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, mproperties)};

        VK_CHECK(vkAllocateMemory(device_, &allocInfo, nullptr, &imageMemory), "failed to allocate image memory!");

        VK_CHECK(vkBindImageMemory(device_, image, imageMemory, 0), "failed to bind image memory!");
    }
    DISABLE_WARNINGS_POP()

}  // namespace lve
   // NOLINTEND(*-include-cleaner)
