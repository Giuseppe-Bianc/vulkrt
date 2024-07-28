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

        [[nodiscard]] glm::mat4 mat4() const;

        [[nodiscard]] glm::mat4 normalMatrix() const;

        // NOLINTBEGIN(*-pro-type-union-access)

        // NOLINTEND(*-pro-type-union-access)
    };

    class GameObject {
    public:
        using id_t = unsigned int;
        static GameObject createGameObject() noexcept {
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
