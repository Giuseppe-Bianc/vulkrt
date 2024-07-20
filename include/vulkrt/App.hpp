//
// Created by gbian on 15/07/2024.
//
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "GameObject.hpp"
#include "Model.hpp"
#include "Pipeline.hpp"
#include "SwapChain.hpp"
#include "Window.hpp"

namespace lve {
    DISABLE_WARNINGS_PUSH(26426)
    static inline const auto curent = fs::current_path();
    DISABLE_WARNINGS_POP()

    DISABLE_WARNINGS_PUSH(4324)
    struct SimplePushConstantData {
        glm::mat2 transform{1.0f};
        glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };
    DISABLE_WARNINGS_POP()

    class App {
    public:
        App() noexcept;
        ~App();
        App(const App &) = delete;
        App &operator=(const App &) = delete;
        void run();

    private:
        void loadGameObjects();
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void freeCommandBuffers() noexcept;
        void drawFrame();
        void recreateSwapChain();
        void recordCommandBuffer(int imageIndex);
        void renderGameObjects(VkCommandBuffer commandBuffer);

        Window lveWindow{WWIDTH, WHEIGHT, WTITILE};
        Device lveDevice{lveWindow};
        std::unique_ptr<SwapChain> lveSwapChain;
        std::unique_ptr<Pipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<GameObject> gameObjects;
    };
}  // namespace lve
// NOLINTEND(*-include-cleaner)
