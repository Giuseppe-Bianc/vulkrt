//
// Created by gbian on 17/07/2024.
//

#pragma once

#include "Device.hpp"

namespace lve {

    class Model {
    public:
        struct Vertex {
            glm::vec3 position;
            glm::vec3 color;
            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        };
        struct Builder {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};
        };

        Model(Device &device, const Model::Builder &builder) noexcept;
        ~Model();
        Model(const Model &) = delete;
        Model &operator=(const Model &) = delete;

        void bind(VkCommandBuffer commandBuffer) noexcept;
        void draw(VkCommandBuffer commandBuffer) const noexcept;

    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<uint32_t> &indices);

        Device &lveDevice;

        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        uint32_t vertexCount;

        bool hasIndexBuffer = false;
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;
        uint32_t indexCount;
    };

}  // namespace lve
