//
// Created by gbian on 20/07/2024.
//

#pragma once
#include "Camera.hpp"
#include "Device.hpp"
#include "FrameInfo.hpp"
#include "GameObject.hpp"
#include "Pipeline.hpp"

namespace lve {

    class SimpleRenderSystem {
    public:
        SimpleRenderSystem(Device &device, VkRenderPass renderPass);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem &) = delete;
        SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

        void renderGameObjects(FrameInfo &frameInfo, std::vector<GameObject> &gameObjects);

    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);

        Device &lveDevice;

        std::unique_ptr<Pipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
    };
}  // namespace lve