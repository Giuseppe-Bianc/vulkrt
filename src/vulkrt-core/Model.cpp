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
    static inline constexpr auto defaultColor=glm::vec3{1.f, 1.f, 1.f};
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

    std::unique_ptr<Model> Model::createModelFromFile(Device &device, const std::string &filepath) {
        Builder builder{};
        builder.loadModel(filepath);
        LINFO("{} vertex count: {} ", filepath, builder.vertices.size());
        return MAKE_UNIQUE(Model, device, builder);
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

    void Model::Builder::loadModel(const std::string &filepath) {
#ifdef INDEPTH
#endif
        vnd::AutoTimer t{FORMAT("loadModel {}", filepath), vnd::Timer::Big};
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
        std::for_each(std::execution::par,shapes.begin(), shapes.end(), [&](const auto&  shape) {
            for(const auto &index : shape.mesh.indices) {
                Vertex vertex{};
                const auto vertex_index = 3 * index.vertex_index;

                if(index.vertex_index >= 0) [[likely]] {
                    vertex.position = {attrib.vertices[vertex_index], attrib.vertices[vertex_index + 1],
                                       attrib.vertices[vertex_index + 2]};

                    auto colorIndex = vertex_index + 2;
                    if(colorIndex < attrib.colors.size()) [[likely]] {
                        vertex.color = {attrib.colors[colorIndex - 2], attrib.colors[colorIndex - 1], attrib.colors[colorIndex]};
                    } else  [[unlikely]] {
                        vertex.color =  defaultColor;  // set default color
                    }
                }

                if(index.normal_index >= 0) [[likely]] {
                    const auto normal_index = 3 * index.normal_index;
                    vertex.normal = {attrib.normals[normal_index], attrib.normals[normal_index + 1], attrib.normals[normal_index + 2]};
                }

                if(index.texcoord_index >= 0) [[likely]] {
                    const  auto texcoord_index = 2 * index.texcoord_index;
                    vertex.uv = {attrib.texcoords[texcoord_index], attrib.texcoords[texcoord_index + 1]};
                }

                if(uniqueVertices.count(vertex) == 0) [[likely]] {
                    uniqueVertices[vertex] = C_UI32T(vertices.size());
                    vertices.emplace_back(std::move(vertex));
                }
                indices.emplace_back(uniqueVertices[vertex]);
            }
        });
        /*for(const auto &shape : shapes) {
            for(const auto &index : shape.mesh.indices) {
                Vertex vertex{};
                const auto vertex_index = 3 * index.vertex_index;

                if(index.vertex_index >= 0) [[likely]] {
                    vertex.position = {attrib.vertices[vertex_index], attrib.vertices[vertex_index + 1],
                                       attrib.vertices[vertex_index + 2]};

                    auto colorIndex = vertex_index + 2;
                    if(colorIndex < attrib.colors.size()) [[likely]] {
                        vertex.color = {attrib.colors[colorIndex - 2], attrib.colors[colorIndex - 1], attrib.colors[colorIndex]};
                    } else  [[unlikely]] {
                        vertex.color = {1.f, 1.f, 1.f};  // set default color
                    }
                }

                if(index.normal_index >= 0) [[likely]] {
                    const auto normal_index = 3 * index.normal_index;
                    vertex.normal = {attrib.normals[normal_index], attrib.normals[normal_index + 1], attrib.normals[normal_index + 2]};
                }

                if(index.texcoord_index >= 0) [[likely]] {
                    const  auto texcoord_index = 2 * index.texcoord_index;
                    vertex.uv = {attrib.texcoords[texcoord_index], attrib.texcoords[texcoord_index + 1]};
                }

                if(uniqueVertices.count(vertex) == 0) [[likely]] {
                    uniqueVertices[vertex] = C_UI32T(vertices.size());
                    vertices.emplace_back(std::move(vertex));
                }
                indices.emplace_back(uniqueVertices[vertex]);
            }
        }*/
    }

    DISABLE_WARNINGS_POP()
}  // namespace lve
   // NOLINTEND(*-include-cleaner)