//
// Created by gbian on 17/07/2024.
//
// NOLINTBEGIN(*-include-cleaner)
#include "vulkrt/Model.hpp"
#include "vulkrt/Util.hpp"
#include "vulkrt/tiny_obj_loader.h"

#include <vulkrt/timer/Timer.hpp>

namespace std {
    template <> struct hash<lve::Model::Vertex> {
        size_t operator()(lve::Model::Vertex const &vertex) const noexcept {
            size_t seed = 0;
            lve::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}  // namespace std

namespace lve {
    DISABLE_WARNINGS_PUSH(26432 26447)
    Model::Model(Device &device, const Model::Builder &builder) noexcept : lveDevice{device} {
        createVertexBuffers(builder.vertices);
        createIndexBuffers(builder.indices);
    }
    DISABLE_WARNINGS_POP()
    DISABLE_WARNINGS_PUSH(26446)
    static inline constexpr auto VERTEX_SIZE = sizeof(Model::Vertex);
    std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = VERTEX_SIZE;
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }
    std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.emplace_back(VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
        attributeDescriptions.emplace_back(VkVertexInputAttributeDescription{1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
        attributeDescriptions.emplace_back(VkVertexInputAttributeDescription{2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
        attributeDescriptions.emplace_back(VkVertexInputAttributeDescription{3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

        return attributeDescriptions;
    }
    void Model::bind(VkCommandBuffer commandBuffer) noexcept {
        const std::array<VkBuffer, 1> buffers = {vertexBuffer->getBuffer()};
        const std::array<VkDeviceSize, 1> offsets = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers.data(), offsets.data());
        if(hasIndexBuffer) [[likely]] { vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32); }
    }
    void Model::draw(VkCommandBuffer commandBuffer) const noexcept {
        if(hasIndexBuffer) [[likely]] {
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
        } else [[unlikely]] {
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
        }
    }

    std::unique_ptr<Model> Model::createModelFromFile(Device &device, const std::string &filepath) {
        Builder builder{};
        builder.loadModel(filepath);
        LINFO("{} vertex count: {}", filepath, builder.vertices.size());
        return MAKE_UNIQUE(Model, device, builder);
    }

    void Model::createVertexBuffers(const std::vector<Vertex> &vertices) {
        vertexCount = C_UI32T(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        uint32_t vertexSize = sizeof(vertices[0]);
        const VkDeviceSize bufferSize = static_cast<VkDeviceSize>(vertexSize) * vertexCount;

        // NOLINTBEGIN(*-signed-bitwise)
        Buffer stagingBuffer{
            lveDevice,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer(std::bit_cast<void *>(vertices.data()));

        vertexBuffer = std::make_unique<Buffer>(lveDevice, vertexSize, vertexCount,
                                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        // NOLINTEND(*-signed-bitwise)

        lveDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
    }

    void Model::createIndexBuffers(const std::vector<uint32_t> &indices) {
        indexCount = C_UI32T(indices.size());
        hasIndexBuffer = indexCount > 0;

        if(!hasIndexBuffer) { return; }

        const uint32_t indexSize = sizeof(indices[0]);
        const VkDeviceSize bufferSize = static_cast<VkDeviceSize>(indexSize) * indexCount;

        // NOLINTBEGIN(*-signed-bitwise)
        Buffer stagingBuffer{
            lveDevice,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer(std::bit_cast<void *>(indices.data()));

        indexBuffer = std::make_unique<Buffer>(lveDevice, indexSize, indexCount,
                                               VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        // NOLINTEND(*-signed-bitwise)

        lveDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
    }

    void Model::Builder::loadModel(const std::string &filepath) {
#ifdef INDEPTH
        const vnd::AutoTimer t{FORMAT("loadModel {}", filepath), vnd::Timer::Big};
#endif
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) [[unlikely]] {
            throw std::runtime_error(warn + err);
        }

        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        uniqueVertices.reserve(shapes.size() * 3);
        std::for_each(std::execution::par, shapes.begin(), shapes.end(), [&](const auto &shape) {
            for(const auto &index : shape.mesh.indices) {
                Vertex vertex{};
                const auto vertex_index = 3 * index.vertex_index;

                if(index.vertex_index >= 0) [[likely]] {
                    vertex.position = {attrib.vertices[vertex_index], attrib.vertices[vertex_index + 1], attrib.vertices[vertex_index + 2]};

                    vertex.color = {attrib.colors[vertex_index], attrib.colors[vertex_index + 1], attrib.colors[vertex_index + 2]};
                }

                if(index.normal_index >= 0) [[likely]] {
                    const auto normal_index = 3 * index.normal_index;
                    vertex.normal = {attrib.normals[normal_index], attrib.normals[normal_index + 1], attrib.normals[normal_index + 2]};
                }

                if(index.texcoord_index >= 0) [[likely]] {
                    const auto texcoord_index = 2 * index.texcoord_index;
                    vertex.uv = {attrib.texcoords[texcoord_index], attrib.texcoords[texcoord_index + 1]};
                }

                if(uniqueVertices.count(vertex) == 0) [[likely]] {
                    uniqueVertices[vertex] = C_UI32T(vertices.size());
                    vertices.emplace_back(std::move(vertex));
                }
                indices.emplace_back(uniqueVertices[vertex]);
            }
        });
    }

    DISABLE_WARNINGS_POP()
}  // namespace lve
   // NOLINTEND(*-include-cleaner)
