#include "pipeline.hpp"

#include <cassert>
#include <fstream>
#include <stdexcept>

#include "model.hpp"

Pipeline::Pipeline(Device& device, const char* vertFilePath, const char* fragFilePath,
    const PipelineConfigInfo& configInfo)
    : _device{device} {
    CreateGraphicsPipeline(vertFilePath, fragFilePath, configInfo);
}

Pipeline::~Pipeline() {
    vkDestroyShaderModule(_device.GetDevice(), _vertShaderModule, nullptr);
    vkDestroyShaderModule(_device.GetDevice(), _fragShaderModule, nullptr);
    vkDestroyPipeline(_device.GetDevice(), _graphicsPipeline, nullptr);
}

void Pipeline::Bind(VkCommandBuffer commandBuffer) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);
}

void Pipeline::DefaultPipelineConfigInfo(
    PipelineConfigInfo& configInfo, uint32_t width, uint32_t height) {
    configInfo.InputAssemblyInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    configInfo.InputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    configInfo.InputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    configInfo.Viewport.x = 0.0f;
    configInfo.Viewport.y = 0.0f;
    configInfo.Viewport.width = static_cast<float>(width);
    configInfo.Viewport.height = static_cast<float>(height);
    configInfo.Viewport.minDepth = 0.0f;
    configInfo.Viewport.maxDepth = 1.0f;

    configInfo.Scissor.offset = {0, 0};
    configInfo.Scissor.extent = {width, height};

    configInfo.ViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    configInfo.ViewportInfo.viewportCount = 1;
    configInfo.ViewportInfo.pViewports = &configInfo.Viewport;
    configInfo.ViewportInfo.scissorCount = 1;
    configInfo.ViewportInfo.pScissors = &configInfo.Scissor;

    configInfo.RasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    configInfo.RasterizationInfo.depthClampEnable = VK_FALSE;
    configInfo.RasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    configInfo.RasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    configInfo.RasterizationInfo.lineWidth = 1.0f;
    configInfo.RasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    configInfo.RasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    configInfo.RasterizationInfo.depthBiasEnable = VK_FALSE;
    configInfo.RasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
    configInfo.RasterizationInfo.depthBiasClamp = 0.0f;           // Optional
    configInfo.RasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

    configInfo.MultiSampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    configInfo.MultiSampleInfo.sampleShadingEnable = VK_FALSE;
    configInfo.MultiSampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    configInfo.MultiSampleInfo.minSampleShading = 1.0f;           // Optional
    configInfo.MultiSampleInfo.pSampleMask = nullptr;             // Optional
    configInfo.MultiSampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
    configInfo.MultiSampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

    configInfo.ColorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    configInfo.ColorBlendAttachment.blendEnable = VK_FALSE;
    configInfo.ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    configInfo.ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    configInfo.ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
    configInfo.ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    configInfo.ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    configInfo.ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

    configInfo.ColorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    configInfo.ColorBlendInfo.logicOpEnable = VK_FALSE;
    configInfo.ColorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
    configInfo.ColorBlendInfo.attachmentCount = 1;
    configInfo.ColorBlendInfo.pAttachments = &configInfo.ColorBlendAttachment;
    configInfo.ColorBlendInfo.blendConstants[0] = 0.0f;  // Optional
    configInfo.ColorBlendInfo.blendConstants[1] = 0.0f;  // Optional
    configInfo.ColorBlendInfo.blendConstants[2] = 0.0f;  // Optional
    configInfo.ColorBlendInfo.blendConstants[3] = 0.0f;  // Optional

    configInfo.DepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    configInfo.DepthStencilInfo.depthTestEnable = VK_TRUE;
    configInfo.DepthStencilInfo.depthWriteEnable = VK_TRUE;
    configInfo.DepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    configInfo.DepthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    configInfo.DepthStencilInfo.minDepthBounds = 0.0f;  // Optional
    configInfo.DepthStencilInfo.maxDepthBounds = 1.0f;  // Optional
    configInfo.DepthStencilInfo.stencilTestEnable = VK_FALSE;
    configInfo.DepthStencilInfo.front = {};  // Optional
    configInfo.DepthStencilInfo.back = {};   // Optional
}

std::vector<char> Pipeline::ReadFile(const char* filePath) {
    // std::string enginePath = ENGINE_DIR + filepath;

    std::ifstream file{filePath, std::ios::ate | std::ios::binary};

    if (!file.is_open()) throw std::runtime_error("failed to open file: " + *filePath);

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}

void Pipeline::CreateGraphicsPipeline(
    const char* vertPath, const char* fragPath, const PipelineConfigInfo& configInfo) {
    assert(configInfo.PipelineLayout != nullptr &&
           "Cannot create graphics pipeline: no PipelineLayout provided in config info");
    assert(configInfo.RenderPass != nullptr &&
           "Cannot create graphics pipeline: no RenderPass provided in config info");

    std::vector<char> vertCode = ReadFile(vertPath);
    std::vector<char> fragCode = ReadFile(fragPath);

    CreateShaderModule(vertCode, &_vertShaderModule);
    CreateShaderModule(fragCode, &_fragShaderModule);

    VkPipelineShaderStageCreateInfo shaderStages[2];
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = _vertShaderModule;
    shaderStages[0].pName = "main";
    shaderStages[0].flags = 0;
    shaderStages[0].pNext = nullptr;
    shaderStages[0].pSpecializationInfo = nullptr;

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = _fragShaderModule;
    shaderStages[1].pName = "main";
    shaderStages[1].flags = 0;
    shaderStages[1].pNext = nullptr;
    shaderStages[1].pSpecializationInfo = nullptr;

    std::vector<VkVertexInputBindingDescription> bindingDescriptions =
        Model::Vertex::GetBindingDescriptions();
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions =
        Model::Vertex::GetAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount =
        static_cast<uint32_t>(bindingDescriptions.size());
    vertexInputInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &configInfo.InputAssemblyInfo;
    pipelineInfo.pViewportState = &configInfo.ViewportInfo;
    pipelineInfo.pRasterizationState = &configInfo.RasterizationInfo;
    pipelineInfo.pMultisampleState = &configInfo.MultiSampleInfo;
    pipelineInfo.pColorBlendState = &configInfo.ColorBlendInfo;
    pipelineInfo.pDepthStencilState = &configInfo.DepthStencilInfo;
    pipelineInfo.pDynamicState = nullptr;  // Optional

    pipelineInfo.layout = configInfo.PipelineLayout;
    pipelineInfo.renderPass = configInfo.RenderPass;
    pipelineInfo.subpass = configInfo.SubPass;

    pipelineInfo.basePipelineIndex = -1;               // Optional
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional

    if (vkCreateGraphicsPipelines(_device.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
            &_graphicsPipeline) != VK_SUCCESS)
        throw std::runtime_error("failed to create graphics pipeline!");
}

void Pipeline::CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    if (vkCreateShaderModule(_device.GetDevice(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
        throw std::runtime_error("failed to create shader module");
}
