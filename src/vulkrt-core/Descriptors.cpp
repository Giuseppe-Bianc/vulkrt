//
// Created by gbian on 30/07/2024.
//
// NOLINTBEGIN(*-include-cleaner)
#include "vulkrt/Descriptors.hpp"

namespace lve {

    // *************** Descriptor Set Layout Builder *********************

    DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::addBinding(uint32_t binding, VkDescriptorType descriptorType,
                                                                           VkShaderStageFlags stageFlags, uint32_t count) {
        assert(bindings.count(binding) == 0 && "Binding already in use");
        const VkDescriptorSetLayoutBinding layoutBinding{
            .binding = binding,
            .descriptorType = descriptorType,
            .descriptorCount = count,
            .stageFlags = stageFlags,
        };
        bindings[binding] = layoutBinding;
        return *this;
    }

    std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const {
        return MAKE_UNIQUE(DescriptorSetLayout, lveDevice, bindings);
    }

    // *************** Descriptor Set Layout *********************

    DescriptorSetLayout::DescriptorSetLayout(Device &Device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
      : lveDevice{Device}, bindings{bindings} {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for(const auto &kv : bindings) { setLayoutBindings.emplace_back(kv.second); }

        const VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = C_UI32T(setLayoutBindings.size()),
            .pBindings = setLayoutBindings.data(),
        };

        VK_CHECK(vkCreateDescriptorSetLayout(lveDevice.device(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout),
                 "failed to create descriptor set layout!");
    }

    DISABLE_WARNINGS_PUSH(26432)
    DescriptorSetLayout::~DescriptorSetLayout() { vkDestroyDescriptorSetLayout(lveDevice.device(), descriptorSetLayout, nullptr); }
    DISABLE_WARNINGS_POP()

    // *************** Descriptor Pool Builder *********************

    DescriptorPool::Builder &DescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, uint32_t count) {
        poolSizes.push_back({descriptorType, count});
        return *this;
    }

    DescriptorPool::Builder &DescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags) noexcept {
        poolFlags = flags;
        return *this;
    }
    DescriptorPool::Builder &DescriptorPool::Builder::setMaxSets(uint32_t count) noexcept {
        maxSets = count;
        return *this;
    }

    std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const {
        return MAKE_UNIQUE(DescriptorPool, lveDevice, maxSets, poolFlags, poolSizes);
    }

    // *************** Descriptor Pool *********************

    DescriptorPool::DescriptorPool(Device &Device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags,
                                   const std::vector<VkDescriptorPoolSize> &poolSizes)
      : lveDevice{Device} {
        const VkDescriptorPoolCreateInfo descriptorPoolInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = poolFlags,
            .maxSets = maxSets,
            .poolSizeCount = C_UI32T(poolSizes.size()),
            .pPoolSizes = poolSizes.data(),
        };

        VK_CHECK(vkCreateDescriptorPool(Device.device(), &descriptorPoolInfo, nullptr, &descriptorPool),
                 "failed to create descriptor pool!");
    }
    DISABLE_WARNINGS_PUSH(26432)
    DescriptorPool::~DescriptorPool() { vkDestroyDescriptorPool(lveDevice.device(), descriptorPool, nullptr); }
    DISABLE_WARNINGS_POP()

    bool DescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const noexcept {
        const VkDescriptorSetAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &descriptorSetLayout,
        };

        // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
        // a new pool whenever an old pool fills up. But this is beyond our current scope
        if(vkAllocateDescriptorSets(lveDevice.device(), &allocInfo, &descriptor) != VK_SUCCESS) { return false; }
        return true;
    }

    void DescriptorPool::freeDescriptors(const std::vector<VkDescriptorSet> &descriptors) const noexcept {
        vkFreeDescriptorSets(lveDevice.device(), descriptorPool, C_UI32T(descriptors.size()), descriptors.data());
    }

    void DescriptorPool::resetPool() noexcept { vkResetDescriptorPool(lveDevice.device(), descriptorPool, 0); }

    // *************** Descriptor Writer *********************

    DescriptorWriter::DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool) noexcept : setLayout{setLayout}, pool{pool} {}

    DescriptorWriter &DescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo const *bufferInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto &bindingDescription = setLayout.bindings[binding];

        assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstBinding = binding,
            .descriptorCount = 1,
            .descriptorType = bindingDescription.descriptorType,
            .pBufferInfo = bufferInfo,
        };

        writes.emplace_back(write);
        return *this;
    }

    DescriptorWriter &DescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo const *imageInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto &bindingDescription = setLayout.bindings[binding];

        assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

        const VkWriteDescriptorSet write{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstBinding = binding,
            .descriptorCount = 1,
            .descriptorType = bindingDescription.descriptorType,
            .pImageInfo = imageInfo,
        };

        writes.emplace_back(write);
        return *this;
    }

    bool DescriptorWriter::build(VkDescriptorSet &set) noexcept {
        const bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
        if(!success) { return false; }
        overwrite(set);
        return true;
    }

    void DescriptorWriter::overwrite(const VkDescriptorSet &set) noexcept {
        for(auto &write : writes) { write.dstSet = set; }
        vkUpdateDescriptorSets(pool.lveDevice.device(), C_UI32T(writes.size()), writes.data(), 0, nullptr);
    }

}  // namespace lve
// NOLINTEND(*-include-cleaner)
