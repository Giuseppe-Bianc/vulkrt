//
// Created by gbian on 17/07/2024.
//

#pragma once

#include "Device.hpp"

namespace lve {


    class Model {
    public:
        struct Vertex {
            glm::vec2 position;
            glm::vec3 color;
            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        };
        Model(Device &device, const std::vector<Vertex> &vertices) noexcept;
        ~Model();
        Model(const Model &) = delete;
        Model &operator=(const Model &) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);

        Device &lveDevice;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        uint32_t vertexCount;
    };

}  // namespace lve
