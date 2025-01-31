//
// Created by gbian on 21/07/2024.
//

#include "vulkrt/KeyboardMovementController.hpp"
namespace lve {
    static inline constexpr glm::vec3 upDir{0.f, -1.f, 0.f};
    static inline constexpr auto GLM_TWO_PI = glm::two_pi<float>();
    static inline constexpr auto ROATATION_UNIT = 1.0;
    void KeyboardMovementController::moveInPlaneXZ(GLFWwindow *window, float dt, GameObject &gameObject) const {
        glm::vec3 rotate{0};
        // NOLINTBEGIN(*-pro-type-union-access)
        if(glfwGetKey(window, keys.lookRight) == GLFW_PRESS) { rotate.y += ROATATION_UNIT; }
        if(glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) { rotate.y -= ROATATION_UNIT; }
        if(glfwGetKey(window, keys.lookUp) == GLFW_PRESS) { rotate.x += ROATATION_UNIT; }
        if(glfwGetKey(window, keys.lookDown) == GLFW_PRESS) {rotate.x -= ROATATION_UNIT;}
        if(glm::dot(rotate, rotate) > FLOATEPS) { gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate); }

        // limit pitch values between about +/- 85ish degrees
        gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
        gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, GLM_TWO_PI);

        const auto yaw = gameObject.transform.rotation.y;
        const glm::vec3 forwardDir{std::sin(yaw), 0.f, std::cos(yaw)};
        const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};

        glm::vec3 moveDir{0.f};
        if(glfwGetKey(window, keys.moveForward) == GLFW_PRESS) { moveDir += forwardDir; }
        if(glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) { moveDir -= forwardDir; }
        if(glfwGetKey(window, keys.moveRight) == GLFW_PRESS) { moveDir += rightDir; }
        if(glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) { moveDir -= rightDir; }
        if(glfwGetKey(window, keys.moveUp) == GLFW_PRESS) { moveDir += upDir; }
        if(glfwGetKey(window, keys.moveDown) == GLFW_PRESS) { moveDir -= upDir; }

        if(glm::dot(moveDir, moveDir) > FLOATEPS) { gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir); }
        // NOLINTEND(*-pro-type-union-access)
    }
}  // namespace lve
