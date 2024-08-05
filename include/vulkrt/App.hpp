//
// Created by gbian on 15/07/2024.
//
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "GameObject.hpp"
#include "Renderer.hpp"
#include "Window.hpp"
#include "Descriptors.hpp"

namespace lve {

    class App {
    public:
        App() noexcept;
        ~App() = default;
        App(const App &) = delete;
        App &operator=(const App &) = delete;
        void run();

    private:
        void loadGameObjects();
        void updateFrameRate(const float &frametime);
        Window lveWindow{WWIDTH, WHEIGHT, WTITILE};
        Device lveDevice{lveWindow};
        Renderer lveRenderer{lveWindow, lveDevice};
        // note: order of declarations matters
        std::unique_ptr<DescriptorPool> globalPool{};
        GameObject::Map gameObjects;
        int frameCount;
        float totalTime;
    };
}  // namespace lve
// NOLINTEND(*-include-cleaner)
