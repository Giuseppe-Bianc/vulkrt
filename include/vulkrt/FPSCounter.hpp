//
// Created by gbian on 03/07/2024.
//
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "headers.hpp"
#include <GLFW/glfw3.h>
class FPSCounter {
public:
    explicit FPSCounter(GLFWwindow *window, std::string_view title = "title") noexcept;
    void frame();
    void frameInTitle();
    void updateFPS() noexcept;
    [[nodiscard]] long double getFPS() const noexcept;
    [[nodiscard]] long double getFrameTime() const noexcept { return frameTime; };
    [[nodiscard]] long double getMsPerFrame() const noexcept;

private:
    [[nodiscard]] std::string transformTime(const long double inputTimeMilli) const noexcept;
    using clock = ch::high_resolution_clock;
    ch::time_point<clock> last_time;
    int frames;
    long double fps;
    long double ms_per_frame;
    long double totalTime{};
    long double frameTime{};
    GLFWwindow *m_window;
    std::string_view m_title;
    std::string ms_per_frameComposition;
};
// NOLINTEND(*-include-cleaner)
