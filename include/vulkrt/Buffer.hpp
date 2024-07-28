//
// Created by gbian on 28/07/2024.
//

#pragma once
#include "Device.hpp"

namespace lve {

    class Buffer {
    public:
        Buffer(Device &device, VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags,
               VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment = 1);
        ~Buffer();

        Buffer(const Buffer &) = delete;
        Buffer &operator=(const Buffer &) = delete;

        VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) noexcept;
        void unmap() noexcept;

        void writeToBuffer(void *data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) noexcept;
        VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) noexcept;
        [[nodiscard]] VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) noexcept;
        [[nodiscard]] VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) noexcept;

        void writeToIndex(void *data, int index) noexcept;
        VkResult flushIndex(int index) noexcept;
        [[nodiscard]] VkDescriptorBufferInfo descriptorInfoForIndex(int index) noexcept;
        [[nodiscard]] VkResult invalidateIndex(int index) noexcept;

        [[nodiscard]] VkBuffer getBuffer() const noexcept { return buffer; }
        [[nodiscard]] void *getMappedMemory() const noexcept { return mapped; }
        [[nodiscard]] uint32_t getInstanceCount() const noexcept { return instanceCount; }
        [[nodiscard]] VkDeviceSize getInstanceSize() const noexcept { return instanceSize; }
        [[nodiscard]] VkDeviceSize getAlignmentSize() const noexcept { return instanceSize; }
        [[nodiscard]] VkBufferUsageFlags getUsageFlags() const noexcept { return usageFlags; }
        [[nodiscard]] VkMemoryPropertyFlags getMemoryPropertyFlags() const noexcept { return memoryPropertyFlags; }
        [[nodiscard]] VkDeviceSize getBufferSize() const noexcept { return bufferSize; }

    private:
        static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) noexcept;

        Device &lveDevice;
        void *mapped = nullptr;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;

        VkDeviceSize bufferSize;
        uint32_t instanceCount;
        VkDeviceSize instanceSize;
        VkDeviceSize alignmentSize;
        VkBufferUsageFlags usageFlags;
        VkMemoryPropertyFlags memoryPropertyFlags;
    };

}  // namespace lve
