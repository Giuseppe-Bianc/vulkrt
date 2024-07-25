//
// Created by gbian on 17/07/2024.
//
// NOLINTBEGIN(*-include-cleaner)
#include "vulkrt/Model.hpp"

namespace lve {
    DISABLE_WARNINGS_PUSH(26432 26447)
    Model::Model(Device &device, const Model::Builder &builder) noexcept : lveDevice{device} {
        createVertexBuffers(builder.vertices);
        createIndexBuffers(builder.indices);
    }
    Model::~Model() {
        const auto device_device = lveDevice.device();
        vkDestroyBuffer(device_device, vertexBuffer, nullptr);
        vkFreeMemory(device_device, vertexBufferMemory, nullptr);
        if(hasIndexBuffer) {
            vkDestroyBuffer(device_device, indexBuffer, nullptr);
            vkFreeMemory(device_device, indexBufferMemory, nullptr);
        }
    }
    DISABLE_WARNINGS_POP()
    DISABLE_WARNINGS_PUSH(26446)
    std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }
    std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);
        return attributeDescriptions;
    }
    void Model::bind(VkCommandBuffer commandBuffer) noexcept {
        const std::vector<VkBuffer> buffers = {vertexBuffer};
        const std::vector<VkDeviceSize> offsets = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers.data(), offsets.data());
        if(hasIndexBuffer) { vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32); }
    }
    void Model::draw(VkCommandBuffer commandBuffer) const noexcept {
        if(hasIndexBuffer) {
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
        } else {
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
        }
    }
    void Model::createVertexBuffers(const std::vector<Vertex> &vertices) {
        vertexCount = C_UI32T(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        const VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        const auto device_device = lveDevice.device();
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        lveDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                               stagingBufferMemory);

        void *data;
        vkMapMemory(device_device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), C_ST(bufferSize));
        vkUnmapMemory(device_device, stagingBufferMemory);

        lveDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
        lveDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

        vkDestroyBuffer(device_device, stagingBuffer, nullptr);
        vkFreeMemory(device_device, stagingBufferMemory, nullptr);
    }

    void Model::createIndexBuffers(const std::vector<uint32_t> &indices) {
        indexCount = C_UI32T(indices.size());
        hasIndexBuffer = indexCount > 0;

        if(!hasIndexBuffer) [[unlikely]] { return; }

        const VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
        const auto device_device = lveDevice.device();

        VkBuffer stagingBuffer{};
        VkDeviceMemory stagingBufferMemory{};
        lveDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                               stagingBufferMemory);
        void *data{};
        vkMapMemory(device_device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), C_ST(bufferSize));
        vkUnmapMemory(device_device, stagingBufferMemory);
        lveDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
        lveDevice.copyBuffer(stagingBuffer, indexBuffer, bufferSize);
        vkDestroyBuffer(device_device, stagingBuffer, nullptr);
        vkFreeMemory(device_device, stagingBufferMemory, nullptr);
    }
    DISABLE_WARNINGS_POP()
}  // namespace lve
   // NOLINTEND(*-include-cleaner)