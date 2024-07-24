//
// Created by gbian on 15/07/2024.
//
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "GameObject.hpp"
#include "Renderer.hpp"
#include "Window.hpp"

namespace lve {

    class App {
    public:
        App() noexcept;
        ~App();
        App(const App &) = delete;
        App &operator=(const App &) = delete;
        void run();

    private:
        void loadGameObjects();
        void updateFrameRate(const float &frametime);
        Window lveWindow{WWIDTH, WHEIGHT, WTITILE};
        Device lveDevice{lveWindow};
        Renderer lveRenderer{lveWindow, lveDevice};
        std::vector<GameObject> gameObjects;
        int frameCount;
        float totalTime;
    };
}  // namespace lve
// NOLINTEND(*-include-cleaner)
