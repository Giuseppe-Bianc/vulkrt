//
// Created by gbian on 19/07/2024.
//
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "Model.hpp"

namespace lve {

    struct Transform2dComponent {
        glm::vec2 translation{};  // (position offset)
        glm::vec2 scale{1.f, 1.f};
        float rotation;

        glm::mat2 mat2() {
            const float s = glm::sin(rotation);
            const float c = glm::cos(rotation);
            const glm::mat2 rotMatrix{{c, s}, {-s, c}};

            const glm::mat2 scaleMat{{scale.x, .0f}, {.0f, scale.y}};
            return rotMatrix * scaleMat;
        }
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
        Transform2dComponent transform2d{};

    private:
        explicit GameObject(const id_t objId) : id(objId) {}
        id_t id;
    };

}  // namespace lve
   // NOLINTEND(*-include-cleaner)