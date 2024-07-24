//
// Created by gbian on 20/07/2024.
//

#include "../../include/vulkrt/SimpleRenderSystem.hpp"

#include <vulkrt/timer/Timer.hpp>

namespace lve {
    DISABLE_WARNINGS_PUSH(4324)
    struct SimplePushConstantData {
        glm::mat4 transform{1.0f};
        alignas(16) glm::vec3 color;
    };
    DISABLE_WARNINGS_POP()
    DISABLE_WARNINGS_PUSH(26432 26447)
    SimpleRenderSystem::SimpleRenderSystem(Device &device, VkRenderPass renderPass) : lveDevice{device} {
        createPipelineLayout();
        createPipeline(renderPass);
    }

    SimpleRenderSystem::~SimpleRenderSystem() { vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr); }
    DISABLE_WARNINGS_POP()

    /*void sierpinski(
    std::vector<Model::Vertex> &vertices,
    int depth,
    const Model::Vertex& left,
    const Model::Vertex& right,
    const Model::Vertex& top) {
        if (depth <= 0) {
            vertices.emplace_back(top);
            vertices.emplace_back(right);
            vertices.emplace_back(left);
        } else {
            auto leftTopP = 0.5f * (left.position + top.position);
            auto rightTopP = 0.5f * (right.position + top.position);
            auto leftRightP = 0.5f * (left.position + right.position);
            auto leftTopC = 0.5f * (left.color + top.color);
            auto rightTopC = 0.5f * (right.color + top.color);
            auto leftRightC = 0.5f * (left.color + right.color);
            sierpinski(vertices, depth - 1, left, {leftRightP, leftRightC}, {leftTopP,leftTopC});
            sierpinski(vertices, depth - 1, {leftRightP, leftRightC}, right, {rightTopP, rightTopC});
            sierpinski(vertices, depth - 1, {leftTopP, leftTopC}, {rightTopP, rightTopC}, top);
        }
    }*/

    void SimpleRenderSystem::createPipelineLayout() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        VK_CHECK(vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout),
                 "failed to  create pipeline layout!");
    }

    void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        const auto vertPath = Window::calculateRelativePathToSrcRes(curent, "simple_shader.vert.spv").string();
        // TODO: return to .frag.vert
        const auto fragPath = Window::calculateRelativePathToSrcRes(curent, "simple_shader.frag.spv").string();
        lvePipeline = MAKE_UNIQUE(Pipeline, lveDevice, vertPath, fragPath, pipelineConfig);
    }

    DISABLE_WARNINGS_PUSH(26429 26432 26461 26446 26485)
    static inline constexpr auto GLM_TWO_PI = glm::two_pi<float>();
    static inline constexpr float DELTA_Y = 0.01F;
    static inline constexpr float DELAT_X = 0.005f;

    void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject> &gameObjects, const Camera &camera) {
        lvePipeline->bind(commandBuffer);

        const auto projectionView = camera.getProjection() * camera.getView();

        for(auto &obj : gameObjects) {
            SimplePushConstantData push{};
            push.color = obj.color;

            push.transform = projectionView * obj.transform.mat4();

            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                               sizeof(SimplePushConstantData), &push);
            obj.model->bind(commandBuffer);
            obj.model->draw(commandBuffer);
        }
    }
    DISABLE_WARNINGS_POP()

}  // namespace lve
   // NOLINTEND(*-include-cleaner)
