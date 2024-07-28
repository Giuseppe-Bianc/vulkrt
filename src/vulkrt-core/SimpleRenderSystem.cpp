//
// Created by gbian on 20/07/2024.
//

#include "vulkrt/SimpleRenderSystem.hpp"

#include <vulkrt/timer/Timer.hpp>

namespace lve {
    DISABLE_WARNINGS_PUSH(4324)
    struct SimplePushConstantData {
        glm::mat4 transform{1.0F};
        glm::mat4 normalMatrix{1.0F};
    };
    DISABLE_WARNINGS_POP()
    DISABLE_WARNINGS_PUSH(26432 26447)
    SimpleRenderSystem::SimpleRenderSystem(Device &device, VkRenderPass renderPass) : lveDevice{device} {
        createPipelineLayout();
        createPipeline(renderPass);
    }

    SimpleRenderSystem::~SimpleRenderSystem() { vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr); }
    DISABLE_WARNINGS_POP()

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
        const auto vertPath = Window::calculateRelativePathToSrcShaders(curentP, "simple_shader.vert.opt.rmp.spv").string();
        // TODO: return to .frag.vert
        const auto fragPath = Window::calculateRelativePathToSrcShaders(curentP, "simple_shader.frag.opt.rmp.spv").string();
        lvePipeline = MAKE_UNIQUE(Pipeline, lveDevice, vertPath, fragPath, pipelineConfig);
    }

    DISABLE_WARNINGS_PUSH(26429 26432 26461 26446 26485)
    static inline constexpr auto GLM_TWO_PI = glm::two_pi<float>();
    static inline constexpr float DELTA_Y = 0.01F;
    static inline constexpr float DELAT_X = 0.005f;

    void SimpleRenderSystem::renderGameObjects(FrameInfo &frameInfo, std::vector<GameObject> &gameObjects) {
        lvePipeline->bind(frameInfo.commandBuffer);

        const auto projectionView = frameInfo.camera.getProjection() * frameInfo.camera.getView();

        for(auto &obj : gameObjects) {
            SimplePushConstantData push{};
            const auto modelMatrix = obj.transform.mat4();
            push.transform = projectionView * modelMatrix;
            push.normalMatrix = obj.transform.normalMatrix();

            vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                               sizeof(SimplePushConstantData), &push);
            obj.model->bind(frameInfo.commandBuffer);
            obj.model->draw(frameInfo.commandBuffer);
        }
    }

    DISABLE_WARNINGS_POP()

}  // namespace lve
   // NOLINTEND(*-include-cleaner)
