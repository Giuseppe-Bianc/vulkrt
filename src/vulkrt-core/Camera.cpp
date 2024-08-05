//
// Created by gbian on 21/07/2024.
//

#include "vulkrt/Camera.hpp"

namespace lve {
    DISABLE_WARNINGS_PUSH(26446)
    void Camera::setOrthographicProjection(float left, float right, float top, float bottom, float near, float far) {
        projectionMatrix = glm::mat4{1.0f};
        projectionMatrix[0][0] = 2.f / (right - left);
        projectionMatrix[1][1] = 2.f / (bottom - top);
        projectionMatrix[2][2] = 1.f / (far - near);
        projectionMatrix[3][0] = -(right + left) / (right - left);
        projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
        projectionMatrix[3][2] = -near / (far - near);
    }

    void Camera::setPerspectiveProjection(float fovy, float aspect, float near, float far) {
        assert(glm::abs(aspect - FLOATEPS) > 0.0f);
        const float tanHalfFovy = std::tan(fovy / 2.f);
        const float fnDifference = far - near;

        projectionMatrix = glm::mat4(0.f);
        projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
        projectionMatrix[1][1] = 1.f / tanHalfFovy;
        projectionMatrix[2][2] = far / fnDifference;
        projectionMatrix[2][3] = 1.f;
        projectionMatrix[3][2] = -(far * near) / fnDifference;
    }

    void Camera::setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
        const glm::vec3 w{glm::normalize(direction)};
        const glm::vec3 u{glm::normalize(glm::cross(w, up))};
        const glm::vec3 v{glm::cross(w, u)};

        viewMatrix = glm::mat4{1.f};
        viewMatrix[0][0] = u.x;
        viewMatrix[1][0] = u.y;
        viewMatrix[2][0] = u.z;
        viewMatrix[0][1] = v.x;
        viewMatrix[1][1] = v.y;
        viewMatrix[2][1] = v.z;
        viewMatrix[0][2] = w.x;
        viewMatrix[1][2] = w.y;
        viewMatrix[2][2] = w.z;
        viewMatrix[3][0] = -glm::dot(u, position);
        viewMatrix[3][1] = -glm::dot(v, position);
        viewMatrix[3][2] = -glm::dot(w, position);
    }

    void Camera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up) { setViewDirection(position, target - position, up); }

    void Camera::setViewYXZ(glm::vec3 position, glm::vec3 rotation) {
        // Precompute cosine and sine of rotation angles
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);

        // Precompute products of sines and cosines
        const float c1s2 = c1 * s2;
        const float s1s2 = s1 * s2;
        const float c1s3 = c1 * s3;
        const float s1s3 = s1 * s3;
        const float c1c3 = c1 * c3;
        const float s1c3 = s1 * c3;
        // const float s2s3 = s2 * s3;

        // Compute the u, v, and w vectors
        const glm::vec3 u{c1c3 + s1s2 * s3, c2 * s3, c1s2 * s3 - s1c3};
        const glm::vec3 v{s1s3 * s2 - c1s3, c2 * c3, c1c3 * s2 + s1s3};
        const glm::vec3 w{c2 * s1, -s2, c1 * c2};

        // Initialize the view matrix as an identity matrix
        viewMatrix = glm::mat4{1.f};

        // Set the rotation part of the view matrix
        viewMatrix[0][0] = u.x;
        viewMatrix[1][0] = u.y;
        viewMatrix[2][0] = u.z;
        viewMatrix[0][1] = v.x;
        viewMatrix[1][1] = v.y;
        viewMatrix[2][1] = v.z;
        viewMatrix[0][2] = w.x;
        viewMatrix[1][2] = w.y;
        viewMatrix[2][2] = w.z;

        // Set the translation part of the view matrix
        viewMatrix[3][0] = -glm::dot(u, position);
        viewMatrix[3][1] = -glm::dot(v, position);
        viewMatrix[3][2] = -glm::dot(w, position);
    }

    DISABLE_WARNINGS_POP()
}  // namespace lve
