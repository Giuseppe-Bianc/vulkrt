//
// Created by gbian on 20/07/2024.
//

#include "vulkrt/SimpleRenderSystem.hpp"

#include <vulkrt/timer/Timer.hpp>

namespace lve {
    DISABLE_WARNINGS_PUSH(4324)
    struct SimplePushConstantData {
        glm::mat4 modelMatrix{1.0F};
        glm::mat4 normalMatrix{1.0F};
    };
    DISABLE_WARNINGS_POP()
    DISABLE_WARNINGS_PUSH(26432 26447)
    static inline constexpr auto SIMPLE_PUSH_CONSTANT_DATA_SIZE = sizeof(SimplePushConstantData);
    SimpleRenderSystem::SimpleRenderSystem(Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
      : lveDevice{device} {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    SimpleRenderSystem::~SimpleRenderSystem() { vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr); }
    DISABLE_WARNINGS_POP()

    void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        const VkPushConstantRange pushConstantRange{
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            .offset = 0,
            .size = SIMPLE_PUSH_CONSTANT_DATA_SIZE,
        };

        std::array<VkDescriptorSetLayout, 1> descriptorSetLayouts{globalSetLayout};

        const VkPipelineLayoutCreateInfo pipelineLayoutInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = C_UI32T(descriptorSetLayouts.size()),
            .pSetLayouts = descriptorSetLayouts.data(),
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &pushConstantRange,
        };

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

    void SimpleRenderSystem::SimpleRenderSystem::renderGameObjects(FrameInfo& frameInfo) {
        lvePipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
                                &frameInfo.globalDescriptorSet, 0, nullptr);

        for (auto& kv : frameInfo.gameObjects) {
            auto& obj = kv.second;
            if (obj.model == nullptr) { continue;}
            SimplePushConstantData push{};
            push.modelMatrix = obj.transform.mat4();
            push.normalMatrix = obj.transform.normalMatrix();

            // NOLINTNEXTLINE(*-signed-bitwise)
            vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                               SIMPLE_PUSH_CONSTANT_DATA_SIZE, &push);
            obj.model->bind(frameInfo.commandBuffer);
            obj.model->draw(frameInfo.commandBuffer);
        }
    }

    DISABLE_WARNINGS_POP()

}  // namespace lve
   // NOLINTEND(*-include-cleaner)
