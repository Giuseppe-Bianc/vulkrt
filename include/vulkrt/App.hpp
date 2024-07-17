//
// Created by gbian on 15/07/2024.
//
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "Pipeline.hpp"
#include "SwapChain.hpp"
#include "Window.hpp"
#include "Model.hpp"

namespace lve {

    class App {
    public:
        App() noexcept;
        ~App();
        App(const App &) = delete;
        App &operator=(const App &) = delete;
        void run();

    private:
        void loadModels();
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void drawFrame();

        Window lveWindow{WWIDTH, WHEIGHT, WTITILE};
        Device lveDevice{lveWindow};
        SwapChain lveSwapChain{lveDevice, lveWindow.getExtent()};
        std::unique_ptr<Pipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
        std::unique_ptr<Model> lveModel;
    };
}  // namespace lve
// NOLINTEND(*-include-cleaner)
