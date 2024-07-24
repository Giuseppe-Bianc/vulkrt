//
// Created by gbian on 21/07/2024.
//

#pragma once

#include "GameObject.hpp"
#include "Window.hpp"

namespace lve {
    class KeyboardMovementController {
    public:
        struct KeyMappings {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_E;
            int moveDown = GLFW_KEY_Q;
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
            int keeReset = GLFW_KEY_R;
        };

        void moveInPlaneXZ(GLFWwindow *window, float dt, GameObject &gameObject) const;

        KeyMappings keys{};
        float moveSpeed{3.f};
        float lookSpeed{1.5f};
        inline static glm::vec3 rotation;
        inline static glm::vec3 translation;
    };
}  // namespace lve
