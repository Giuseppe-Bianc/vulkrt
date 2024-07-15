//
// Created by gbian on 15/07/2024.
//
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "Pipeline.hpp"
#include "Window.hpp"

namespace lve {

    class App {
    public:
        void run();

    private:
        Window lveWindow{WWIDTH, WHEIGHT, WTITILE};
        Device lveDevice{lveWindow};
        Pipeline lvePipeline{lveDevice, "../../../shaders/simple_shader.vert.spv", "../../../shaders/simple_shader.vert.spv", Pipeline::defaultPipelineConfigInfo(WWIDTH, WHEIGHT)};
    };

}  // namespace lve
// NOLINTEND(*-include-cleaner)
