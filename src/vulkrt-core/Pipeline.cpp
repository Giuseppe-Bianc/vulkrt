//
// Created by gbian on 15/07/2024.
//
// NOLINTBEGIN(*-include-cleaner *-avoid-do-while)
#include "vulkrt/Pipeline.hpp"
#include "vulkrt/Model.hpp"
#include "vulkrt/timer/Timer.hpp"
namespace lve {

    static inline constexpr auto zrval = 0.0F;
    static inline constexpr const char *vertFragPName = "main";

    DISABLE_WARNINGS_PUSH(26432)
    Pipeline::Pipeline(Device &device, const std::string &vertFilepath, const std::string &fragFilepath,
                       const PipelineConfigInfo &configInfo)
      : lveDevice{device} {
        createGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
    }

    Pipeline::~Pipeline() {
        const auto device_device = lveDevice.device();
        vkDestroyShaderModule(device_device, vertShaderModule, nullptr);
        vkDestroyShaderModule(device_device, fragShaderModule, nullptr);
        vkDestroyPipeline(device_device, graphicsPipeline, nullptr);
    }

    std::vector<char> Pipeline::readFile(const std::string &filename) {
        std::ifstream file{filename, std::ios::ate | std::ios::binary};  // NOLINT(*-signed-bitwise)
#ifdef INDEPTH
        const vnd::AutoTimer timer(FORMAT("reading shader {}", filename));
#endif
        if(!file.is_open()) [[unlikely]] { throw std::runtime_error(FORMAT("failed to open file: {}", filename)); }

        // Determine file size using std::ifstream::seekg and tellg
        const auto fileSize = C_ST(file.tellg());
        if(fileSize == -1) [[unlikely]] { throw std::runtime_error(FORMAT("failed to get file size: {}", filename)); }

        // Allocate buffer and read file contents
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), C_LL(fileSize));

        // Check for read errors
        if(!file) [[unlikely]] { throw std::runtime_error(FORMAT("failed to read file: {}", filename)); }

        // Close the file (automatically done by ifstream destructor, but good practice to explicitly close)
        file.close();

        return buffer;
    }
    DISABLE_WARNINGS_POP()

    DISABLE_WARNINGS_PUSH(26446)
    void Pipeline::createGraphicsPipeline(const std::string &vertFilepath, const std::string &fragFilepath,
                                          const PipelineConfigInfo &configInfo) {
#ifdef INDEPTH
        const vnd::AutoTimer timer{"createGraphicsPipeline", vnd::Timer::Big};
#endif
        assert(configInfo.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline: no pipelineLayout provided in configInfo");
        assert(configInfo.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline: no renderPass provided in configInfo");

        const auto vertCode = readFile(vertFilepath);
        const auto fragCode = readFile(fragFilepath);

#ifdef INDEPTH
        LINFO("Vertex Shader Code Size: {}", vertCode.size());
        LINFO("Fragment Shader Code Size: {}", fragCode.size());
#endif

        createShaderModule(vertCode, &vertShaderModule);
        createShaderModule(fragCode, &fragShaderModule);
        const auto device_device = lveDevice.device();

        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{
            VkPipelineShaderStageCreateInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                            .pNext = nullptr,
                                            .flags = 0,
                                            .stage = VK_SHADER_STAGE_VERTEX_BIT,
                                            .module = vertShaderModule,
                                            .pName = vertFragPName,
                                            .pSpecializationInfo = nullptr},

            VkPipelineShaderStageCreateInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                            .pNext = nullptr,
                                            .flags = 0,
                                            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                                            .module = fragShaderModule,
                                            .pName = vertFragPName,
                                            .pSpecializationInfo = nullptr}};

        const auto bindingDescriptions = Model::Vertex::getBindingDescriptions();
        const auto attributeDescriptions = Model::Vertex::getAttributeDescriptions();
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexAttributeDescriptionCount = C_UI32T(attributeDescriptions.size());
        vertexInputInfo.vertexBindingDescriptionCount = C_UI32T(bindingDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

        const VkGraphicsPipelineCreateInfo pipelineInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount = 2,
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &configInfo.inputAssemblyInfo,
            .pViewportState = &configInfo.viewportInfo,
            .pRasterizationState = &configInfo.rasterizationInfo,
            .pMultisampleState = &configInfo.multisampleInfo,
            .pDepthStencilState = &configInfo.depthStencilInfo,
            .pColorBlendState = &configInfo.colorBlendInfo,
            .pDynamicState = &configInfo.dynamicStateInfo,
            .layout = configInfo.pipelineLayout,
            .renderPass = configInfo.renderPass,
            .subpass = configInfo.subpass,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1,
        };

        // Enable pipeline cache
        VkPipelineCache pipelineCache = VK_NULL_HANDLE;
        VkPipelineCacheCreateInfo pipelineCacheInfo{};
        pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        vkCreatePipelineCache(device_device, &pipelineCacheInfo, nullptr, &pipelineCache);

        VK_CHECK(vkCreateGraphicsPipelines(device_device, pipelineCache, 1, &pipelineInfo, nullptr, &graphicsPipeline),
                 "failed to create graphics pipeline");

        // Destroy pipeline cache after creation
        vkDestroyPipelineCache(device_device, pipelineCache, nullptr);
    }
    DISABLE_WARNINGS_POP()

    void Pipeline::createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule) const {
        const VkShaderModuleCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = code.size(),
            .pCode = C_CPCU32T(code.data()),
        };

        VK_CHECK(vkCreateShaderModule(lveDevice.device(), &createInfo, nullptr, shaderModule), "failed to create shader module");
    }

    void Pipeline::bind(VkCommandBuffer commandBuffer) const noexcept {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    }

    void Pipeline::defaultPipelineConfigInfo(PipelineConfigInfo &configInfo) {
#ifdef INDEPTH
        const vnd::AutoTimer timer("defaultPipelineConfigInfo");
#endif
        configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        configInfo.viewportInfo.viewportCount = 1;
        configInfo.viewportInfo.pViewports = nullptr;
        configInfo.viewportInfo.scissorCount = 1;
        configInfo.viewportInfo.pScissors = nullptr;

        configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
        configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        configInfo.rasterizationInfo.lineWidth = 1.0f;
        configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
        configInfo.rasterizationInfo.depthBiasConstantFactor = zrval;  // Optional
        configInfo.rasterizationInfo.depthBiasClamp = zrval;           // Optional
        configInfo.rasterizationInfo.depthBiasSlopeFactor = zrval;     // Optional

        configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
        configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        configInfo.multisampleInfo.minSampleShading = 1.0f;           // Optional
        configInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
        configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
        configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

        configInfo.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                                         VK_COLOR_COMPONENT_A_BIT;
        configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
        configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
        configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

        configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
        configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
        configInfo.colorBlendInfo.attachmentCount = 1;
        configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
        configInfo.colorBlendInfo.blendConstants[0] = zrval;  // Optional
        configInfo.colorBlendInfo.blendConstants[1] = zrval;  // Optional
        configInfo.colorBlendInfo.blendConstants[2] = zrval;  // Optional
        configInfo.colorBlendInfo.blendConstants[3] = zrval;  // Optional

        configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.minDepthBounds = zrval;  // Optional
        configInfo.depthStencilInfo.maxDepthBounds = 1.0f;   // Optional
        configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.front = {};  // Optional
        configInfo.depthStencilInfo.back = {};   // Optional

        configInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
        configInfo.dynamicStateInfo.dynamicStateCount = C_UI32T(configInfo.dynamicStateEnables.size());
        configInfo.dynamicStateInfo.flags = 0;
    }

}  // namespace lve

// NOLINTEND(*-include-cleaner *-avoid-do-while)
