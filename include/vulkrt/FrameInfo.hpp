//
// Created by gbian on 28/07/2024.
//

#pragma once

#include "Camera.hpp"
#include "GameObject.hpp"

#include "vulkanCheck.hpp"

namespace lve {
    struct FrameInfo {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        Camera &camera;
        VkDescriptorSet globalDescriptorSet;
        GameObject::Map &gameObjects;
    };
}  // namespace lve