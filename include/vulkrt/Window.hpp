//
// Created by gbian on 15/07/2024.
//

#pragma once

// clang-format off

#include  "vulkanCheck.hpp"
#include <GLFW/glfw3.h>

// clang-format on

namespace lve {

    class Window {  // NOLINT(*-special-member-functions)
    public:
        Window(const int w, const int h, const std::string_view &window_name) noexcept;
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        [[nodiscard]] GLFWwindow *getGLFWWindow() const noexcept { return window; }
        [[nodiscard]] bool shouldClose() const noexcept { return glfwWindowShouldClose(window); }
        [[nodiscard]] static fs::path calculateRelativePathToSrcRes(const fs::path &executablePath, const fs::path &targetFile);
        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
        [[nodiscard]] VkExtent2D getExtent() const noexcept { return {C_UI32T(width), C_UI(height)}; }
        [[nodiscard]] bool wasWindowResized() noexcept { return framebufferResized; }
        void resetWindowResizedFlag() noexcept { framebufferResized = false; }

    private:
        void initWindow();

        static void errorCallback(int error, const char *description);
        static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
        static void framebufferResizeCallback(GLFWwindow *window, int width, int height) noexcept;
        void createWindow();

        void setHints() const;
        void initializeGLFW() const;

        [[nodiscard]] std::string formatMode(const GLFWvidmode *mode) const;

        void centerWindow();

        int width;
        int height;
        bool framebufferResized = false;
        std::string_view windowName;
        GLFWwindow *window{nullptr};
    };
}  // namespace lve
