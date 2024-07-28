//
// Created by gbian on 15/07/2024.
//
// NOLINTBEGIN(*-include-cleaner)
#include "vulkrt/App.hpp"

#include "vulkrt/Buffer.hpp"
#include "vulkrt/KeyboardMovementController.hpp"
#include "vulkrt/SimpleRenderSystem.hpp"
#include <vulkrt/FPSCounter.hpp>

namespace lve {

    struct GlobalUbo {
        alignas(16) glm::mat4 projectionView{1.f};
        alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3{1.f, -3.f, -1.f});
    };

    static inline constexpr auto GLOBAL_UBO_SIZE = sizeof(GlobalUbo);

    DISABLE_WARNINGS_PUSH(26432 26447)
    App::App() noexcept { loadGameObjects(); }
    DISABLE_WARNINGS_POP()

    DISABLE_WARNINGS_PUSH(26446)
    void App::run() {
        std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
        std::ranges::for_each(uboBuffers, [&](auto &buffer) {
            buffer = MAKE_UNIQUE(Buffer, lveDevice, GLOBAL_UBO_SIZE, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            buffer->map();
        });

        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};
        Camera camera{};
        auto viewerObject = GameObject::createGameObject();
        const KeyboardMovementController cameraController{};

        FPSCounter fps_counter{lveWindow.getGLFWWindow(), WTITILE};
        while(!lveWindow.shouldClose()) {
            glfwPollEvents();
            fps_counter.frameInTitle();
            const auto frameTime = C_F(fps_counter.getFrameTime());

            cameraController.moveInPlaneXZ(lveWindow.getGLFWWindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            const float aspect = lveRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

            if(auto commandBuffer = lveRenderer.beginFrame()) {
                const int frameIndex = lveRenderer.getFrameIndex();
                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, camera};

                // update
                GlobalUbo ubo{};
                ubo.projectionView = camera.getProjection() * camera.getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(frameInfo, gameObjects);
                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(lveDevice.device());
    }
    DISABLE_WARNINGS_POP()

    void App::loadGameObjects() {
        const auto smooth_vase_path = Window::calculateRelativePathToSrcModels(curentP, "smooth_vase.obj").string();
        const auto flat_vase_path = Window::calculateRelativePathToSrcModels(curentP, "flat_vase.obj").string();
        std::shared_ptr<Model> lveModel = Model::createModelFromFile(lveDevice, flat_vase_path);
        auto flatVase = GameObject::createGameObject();
        flatVase.model = lveModel;
        flatVase.transform.translation = {-.5f, .5f, 2.5f};
        flatVase.transform.scale = {3.f, 1.5f, 3.f};
        gameObjects.push_back(std::move(flatVase));

        lveModel = Model::createModelFromFile(lveDevice, smooth_vase_path);
        auto smoothVase = GameObject::createGameObject();
        smoothVase.model = lveModel;
        smoothVase.transform.translation = {.5f, .5f, 2.5f};
        smoothVase.transform.scale = {3.f, 1.5f, 3.f};
        gameObjects.push_back(std::move(smoothVase));
    }

}  // namespace lve
   // NOLINTEND(*-include-cleaner)
