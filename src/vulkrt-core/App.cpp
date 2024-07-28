//
// Created by gbian on 15/07/2024.
//
// NOLINTBEGIN(*-include-cleaner)
#include "vulkrt/App.hpp"

#include "vulkrt/KeyboardMovementController.hpp"
#include "vulkrt/SimpleRenderSystem.hpp"
#include <vulkrt/FPSCounter.hpp>

namespace lve {
    DISABLE_WARNINGS_PUSH(26432 26447)
    App::App() noexcept { loadGameObjects(); }
    App::~App() {}
    DISABLE_WARNINGS_POP()

    void App::run() {
        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};
        Camera camera{};
        camera.setViewTarget(glm::vec3(-1.f, -2.f, -2.f), glm::vec3(0.f, 0.f, 2.5f));
        auto viewerObject = GameObject::createGameObject();
        const KeyboardMovementController cameraController{};

        FPSCounter fps_counter{lveWindow.getGLFWWindow(), WTITILE};
        while(!lveWindow.shouldClose()) {
            glfwPollEvents();
            fps_counter.frameInTitle();

            cameraController.moveInPlaneXZ(lveWindow.getGLFWWindow(), C_F(fps_counter.getFrameTime()), viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            const float aspect = lveRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);
            // Take care of all GLFW events
            if(auto commandBuffer = lveRenderer.beginFrame()) {
                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }
        }
        vkDeviceWaitIdle(lveDevice.device());
    }

    void App::loadGameObjects() {
        const auto smooth_vase_path = Window::calculateRelativePathToSrcModels(curentP, "smooth_vase.obj").string();
        const std::shared_ptr<Model> lveModel = Model::createModelFromFile(lveDevice, smooth_vase_path);
        auto gameObj = GameObject::createGameObject();
        gameObj.model = lveModel;
        gameObj.transform.translation = {.0f, .0f, 2.5f};
        gameObj.transform.scale = glm::vec3(3.f);
        gameObjects.push_back(std::move(gameObj));
    }

}  // namespace lve
   // NOLINTEND(*-include-cleaner)
