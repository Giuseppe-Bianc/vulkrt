//
// Created by gbian on 15/07/2024.
//

#include "vulkrt/App.hpp"
#include "vulkrt/FPSCounter.hpp"

namespace lve {
    void App::run() {
        FPSCounter fps_counter{lveWindow.getGLFWWindow(), WTITILE};
        while(!lveWindow.shouldClose()) {
            fps_counter.frameInTitle();
            // Take care of all GLFW events
            glfwPollEvents();
        }
    }

}  // namespace lve