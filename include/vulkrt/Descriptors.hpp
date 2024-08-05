//
// Created by gbian on 30/07/2024.
//

#pragma once

#include "Device.hpp"

namespace lve {

    class DescriptorSetLayout {
    public:
        class Builder {
        public:
            explicit Builder(Device &lveDevice) noexcept : lveDevice{lveDevice} {}

            [[nodiscard]] Builder &addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags,
                                              uint32_t count = 1);
            [[nodiscard]] std::unique_ptr<DescriptorSetLayout> build() const;

        private:
            Device &lveDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        DescriptorSetLayout(Device &lveDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~DescriptorSetLayout();
        DescriptorSetLayout(const DescriptorSetLayout &) = delete;
        DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

        [[nodiscard]] VkDescriptorSetLayout getDescriptorSetLayout() const noexcept { return descriptorSetLayout; }

    private:
        Device &lveDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class DescriptorWriter;
    };

    class DescriptorPool {
    public:
        class Builder {
        public:
            explicit Builder(Device &lveDevice) noexcept : lveDevice{lveDevice} {}

            [[nodiscard]] Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            [[nodiscard]] Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags) noexcept;
            [[nodiscard]] Builder &setMaxSets(uint32_t count) noexcept;
            [[nodiscard]] std::unique_ptr<DescriptorPool> build() const;

        private:
            Device &lveDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        DescriptorPool(Device &lveDevice, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags,
                       const std::vector<VkDescriptorPoolSize> &poolSizes);
        ~DescriptorPool();
        DescriptorPool(const DescriptorPool &) = delete;
        DescriptorPool &operator=(const DescriptorPool &) = delete;

        [[nodiscard]] bool allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const noexcept;

        void freeDescriptors(const std::vector<VkDescriptorSet> &descriptors) const noexcept;

        void resetPool() noexcept;

    private:
        Device &lveDevice;
        VkDescriptorPool descriptorPool;

        friend class DescriptorWriter;
    };

    class DescriptorWriter {
    public:
        DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool) noexcept;

        DescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo const *bufferInfo);
        DescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo const *imageInfo);

        bool build(VkDescriptorSet &set) noexcept;
        void overwrite(const VkDescriptorSet &set) noexcept;

    private:
        DescriptorSetLayout &setLayout;
        DescriptorPool &pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}  // namespace lve
