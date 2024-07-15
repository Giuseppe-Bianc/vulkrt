//
// Created by gbian on 03/07/2024.
//
// NOLINTBEGIN(*-include-cleaner)
#include "vulkrt/FPSCounter.hpp"
#include "vulkrt/timer/TimeStep.hpp"

DISABLE_WARNINGS_PUSH(26447)

FPSCounter::FPSCounter(GLFWwindow *window, std::string_view title) noexcept
  : last_time(clock::now()), frames(0), fps(0.0L), ms_per_frame(0.0L), m_window(window), m_title(title) {}

std::string FPSCounter::transformTime(const long double inputTimeMilli) const noexcept {
    const auto &[ms, us, ns] = vnd::ValueLable::calculateTransformTimeMilli(inputTimeMilli);
    return FORMATST("{}ms,{}us,{}ns", ms, us, ns);
}

void FPSCounter::frame() {
    updateFPS();
    LINFO("{:.3LF} fps/{}", fps, ms_per_frame);
}

void FPSCounter::frameInTitle() {
    updateFPS();
    glfwSetWindowTitle(m_window, FORMATST("{} - {:.3LF} fps/{}", m_title, fps, ms_per_frameComposition).c_str());
}

void FPSCounter::updateFPS() noexcept {
    frames++;
    const auto ldframes = C_LD(frames);
    const auto current_time = clock::now();
    const auto &time_step = Timestep(current_time - last_time);
    if(const auto &time_steps = time_step.GetSeconds(); time_steps >= 1.0L) {
        last_time = current_time;
        fps = ldframes / time_steps;
        ms_per_frame = time_step.GetMilliseconds() / ldframes;
        frames = 0;
    }
    ms_per_frameComposition = transformTime(ms_per_frame);
}

long double FPSCounter::getFPS() const noexcept { return fps; }

long double FPSCounter::getMsPerFrame() const noexcept { return ms_per_frame; }

DISABLE_WARNINGS_POP()
// NOLINTEND(*-include-cleaner)
