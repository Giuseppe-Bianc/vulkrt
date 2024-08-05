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

    DISABLE_WARNINGS_PUSH(4324)
    struct GlobalUbo {
        glm::mat4 projectionView{1.f};
        glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f};  // w is intensity
        glm::vec3 lightPosition{-1.f};
        alignas(16) glm::vec4 lightColor{1.f};  // w is light intensity
    };
    DISABLE_WARNINGS_POP()

    static inline constexpr auto GLOBAL_UBO_SIZE = sizeof(GlobalUbo);

    DISABLE_WARNINGS_PUSH(26432 26447)
    App::App() noexcept {
        globalPool = DescriptorPool::Builder(lveDevice)
                         .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                         .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
                         .build();
        loadGameObjects();
    }
    DISABLE_WARNINGS_POP()

    DISABLE_WARNINGS_PUSH(26446)
    void App::run() {
        std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
        std::ranges::for_each(uboBuffers, [&](auto &buffer) {
            buffer = MAKE_UNIQUE(Buffer, lveDevice, GLOBAL_UBO_SIZE, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            buffer->map();
        });

        auto globalSetLayout =
            DescriptorSetLayout::Builder(lveDevice).addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS).build();

        std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < globalDescriptorSets.size(); i++) {
            const auto bufferInfo = uboBuffers[i]->descriptorInfo();
            DescriptorWriter(*globalSetLayout, *globalPool).writeBuffer(0, &bufferInfo).build(globalDescriptorSets[i]);
        }

        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        Camera camera{};
        auto viewerObject = GameObject::createGameObject();
        viewerObject.transform.translation.z = -2.5f;
        const KeyboardMovementController cameraController{};

        FPSCounter fps_counter{lveWindow.getGLFWWindow(), WTITILE};
        while(!lveWindow.shouldClose()) {
            glfwPollEvents();
            fps_counter.frameInTitle();
            const auto frameTime = C_F(fps_counter.getFrameTime());

            cameraController.moveInPlaneXZ(lveWindow.getGLFWWindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            const float aspect = lveRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

            if(auto commandBuffer = lveRenderer.beginFrame()) {
                const int frameIndex = lveRenderer.getFrameIndex();
                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex], gameObjects};

                // update
                GlobalUbo ubo{};
                ubo.projectionView = camera.getProjection() * camera.getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(frameInfo);
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
        const auto quad_path = Window::calculateRelativePathToSrcModels(curentP, "quad.obj").string();
        std::shared_ptr<Model> lveModel = Model::createModelFromFile(lveDevice, flat_vase_path);
        auto flatVase = GameObject::createGameObject();
        flatVase.model = lveModel;
        flatVase.transform.translation = {-.5f, .5f, 0.0f};
        flatVase.transform.scale = {3.f, 1.5f, 3.f};
        gameObjects.emplace(flatVase.get_id(),std::move(flatVase));

        lveModel = Model::createModelFromFile(lveDevice, smooth_vase_path);
        auto smoothVase = GameObject::createGameObject();
        smoothVase.model = lveModel;
        smoothVase.transform.translation = {.5f, .5f, 0.0f};
        smoothVase.transform.scale = {3.f, 1.5f, 3.f};
        gameObjects.emplace(smoothVase.get_id(), std::move(smoothVase));

        lveModel = Model::createModelFromFile(lveDevice, quad_path);
        auto floor = GameObject::createGameObject();
        floor.model = lveModel;
        floor.transform.translation = {0.f, .5f, 0.f};
        floor.transform.scale = {3.f, 1.f, 3.f};
        gameObjects.emplace(floor.get_id(), std::move(floor));
    }

}  // namespace lve
   // NOLINTEND(*-include-cleaner)
