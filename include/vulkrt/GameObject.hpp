//
// Created by gbian on 19/07/2024.
//
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "Model.hpp"

namespace lve {

    struct TransformComponent {
        glm::vec3 translation{};  // (position offset)
        glm::vec3 scale{1.F, 1.F, 1.F};
        glm::vec3 rotation;

        // NOLINTBEGIN(*-pro-type-union-access)
        [[nodiscard]] glm::mat4 mat4() const {
            /* auto transform = glm::translate(glm::mat4{1.0F}, translation);
            transform *= glm::toMat4(glm::quat(rotation));
            transform = glm::scale(transform, scale);
            return transform;*/

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
        // NOLINTEND(*-pro-type-union-access)
    };

    class GameObject {
    public:
        using id_t = unsigned int;
        static GameObject createGameObject() {
            static id_t currentId = 0;
            return GameObject{currentId++};
        }
        [[nodiscard]] id_t get_id() const noexcept { return id; }
        GameObject(const GameObject &) = delete;
        GameObject &operator=(const GameObject &) = delete;
        GameObject(GameObject &&) = default;
        GameObject &operator=(GameObject &&) = default;

        std::shared_ptr<Model> model{};
        glm::vec3 color{};
        TransformComponent transform{};

    private:
        GameObject() noexcept = default;
        explicit GameObject(const id_t objId) noexcept : id(objId) {}

        id_t id;
    };

}  // namespace lve
   // NOLINTEND(*-include-cleaner)