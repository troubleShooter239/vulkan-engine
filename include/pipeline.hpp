#pragma once

#include <vector>

#include "device.hpp"

struct PipelineConfigInfo {
    PipelineConfigInfo() = default;
    PipelineConfigInfo(const PipelineConfigInfo&) = delete;
    PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

    VkViewport Viewport;
    VkRect2D Scissor;
    VkPipelineViewportStateCreateInfo ViewportInfo;
    VkPipelineInputAssemblyStateCreateInfo InputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo RasterizationInfo;
    VkPipelineMultisampleStateCreateInfo MultiSampleInfo;
    VkPipelineColorBlendAttachmentState ColorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo ColorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo DepthStencilInfo;
    VkPipelineLayout PipelineLayout = nullptr;
    VkRenderPass RenderPass = nullptr;
    uint32_t SubPass = 0;
};

class Pipeline {
public:
    Pipeline(Device& device, const char* vertFilePath, const char* fragFilePath,
        const PipelineConfigInfo& configInfo);
    ~Pipeline();

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    void Bind(VkCommandBuffer commandBuffer);
    static void DefaultPipelineConfigInfo(
        PipelineConfigInfo& configInfo, uint32_t width, uint32_t height);

private:
    static std::vector<char> ReadFile(const char* filePath);

    void CreateGraphicsPipeline(
        const char* vertPath, const char* fragPath, const PipelineConfigInfo& configInfo);

    void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

    Device& _device;
    VkPipeline _graphicsPipeline;
    VkShaderModule _vertShaderModule;
    VkShaderModule _fragShaderModule;
};
