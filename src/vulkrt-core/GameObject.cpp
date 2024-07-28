//
// Created by gbian on 28/07/2024.
//
// NOLINTBEGIN(*-include-cleaner)
#include "vulkrt/GameObject.hpp"

namespace lve {
    // NOLINTBEGIN(*-pro-type-union-access)

    glm::mat4 TransformComponent::mat4() const {
        // auto transform = glm::translate(glm::mat4{1.0F}, translation);
        // transform *= glm::toMat4(glm::quat(rotation));
        // transform = glm::scale(transform, scale);
        // return transform;

        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);

        const float mc13 = c1 * c3;
        const float ms23 = s2 * s3;
        const float mcs31 = c3 * s1;

        const float scaleX = scale.x;
        const float scaleY = scale.y;
        const float scaleZ = scale.z;

        // Precompute scale multipliers
        const float sx_mc13 = scaleX * mc13;
        const float sx_s1_ms23 = scaleX * (s1 * ms23);
        const float sy_mcs31_s2 = scaleY * (mcs31 * s2);
        const float sy_c1_s3 = scaleY * (c1 * s3);
        const float sz_c2_s1 = scaleZ * (c2 * s1);
        const float sz_neg_s2 = scaleZ * (-s2);
        const float sz_c1_c2 = scaleZ * (c1 * c2);

        return glm::mat4{{sx_mc13 + sx_s1_ms23, scaleX * c2 * s3, scaleX * (c1 * ms23 - mcs31), 0.0f},
                         {sy_mcs31_s2 - sy_c1_s3, scaleY * c2 * c3, scaleY * (mc13 * s2 + s1 * s3), 0.0f},
                         {sz_c2_s1, sz_neg_s2, sz_c1_c2, 0.0f},
                         {translation.x, translation.y, translation.z, 1.0f}};
    }
    glm::mat4 TransformComponent::normalMatrix() const {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);
        const glm::vec3 invScale = 1.0f / scale;

        const float mc13 = c1 * c3;
        const float ms23 = s2 * s3;
        const float mcs31 = c3 * s1;

        const float invScaleX = invScale.x;
        const float invScaleY = invScale.y;
        const float invScaleZ = invScale.z;

        return glm::mat4{{invScaleX * (mc13 + s1 * ms23), invScaleX * (c2 * s3), invScaleX * (c1 * ms23 - mcs31), 0},
                         {invScaleY * (mcs31 * s2 - c1 * s3), invScaleY * (c2 * c3), invScaleY * (mc13 * s2 + s1 * s3), 0},
                         {invScaleZ * (c2 * s1), invScaleZ * (-s2), invScaleZ * (c1 * c2), 0},
                         {0, 0, 0, 0}};
    }
    // NOLINTEND(*-pro-type-union-access)

}  // namespace lve
   // NOLINTEND(*-include-cleaner)