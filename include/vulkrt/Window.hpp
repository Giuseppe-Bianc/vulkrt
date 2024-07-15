//
// Created by gbian on 15/07/2024.
//

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "headers.hpp"

namespace lve {

    class Window {
    public:
        Window(const int w, const int h, const std::string_view &window_name);
        ~Window();

        Window(const Window &other) = delete;
        Window(Window &&other) noexcept = delete;
        Window &operator=(const Window &other)  = delete;
        Window &operator=(Window &&other) noexcept = delete;

        [[nodiscard]] GLFWwindow *getGLFWWindow() { return window; }
        [[nodiscard]] bool shouldClose() { return glfwWindowShouldClose(window); }

    private:
        void initWindow();

        static void errorCallback(int error, const char *description);
        static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
        void createWindow();

        void setHints();
        void initializeGLFW();

        [[nodiscard]] std::string formatMode(const GLFWvidmode *mode);

        void centerWindow();

        /*fs::path calculateRelativePathToSrcRes(const fs::path &executablePath, const fs::path &targetFile);*/

        const int width;
        const int height;
        std::string_view windowName;
        GLFWwindow *window{nullptr};
    };
}  // namespace lve
