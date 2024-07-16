//
// Created by gbian on 15/07/2024.
//
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "Device.hpp"
#include "headers.hpp"

#include <vulkan/vulkan.h>

namespace lve {

    struct PipelineConfigInfo {
        VkViewport viewport;
        VkRect2D scissor;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class Pipeline {
    public:
        Pipeline(Device &device, const std::string &vertFilepath, const std::string &fragFilepath, const PipelineConfigInfo &configInfo);
        Pipeline(const Pipeline &other) = delete;
        Pipeline &operator=(const Pipeline &other) = delete;
        ~Pipeline();

        void bind(VkCommandBuffer commandBuffer) const noexcept;

        static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height) noexcept;

    private:
        static std::vector<char> readFile(const std::string &filename);

        void createGraphicsPipeline(const std::string &vertFilepath, const std::string &fragFilepath, const PipelineConfigInfo &configInfo);

        void createShaderMolule(const std::vector<char> &code, VkShaderModule *shaderModule);

        Device &lveDevice;
        VkPipeline graphicsPipeline;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;
    };

}  // namespace lve
   // NOLINTEND(*-include-cleaner)
